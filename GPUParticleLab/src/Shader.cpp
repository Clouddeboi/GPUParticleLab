#include "../headers/Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace
{
    std::string ReadFile(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            throw std::runtime_error("Cannot open shader file: " + path);

        std::ostringstream contents;
        contents << file.rdbuf();
        return contents.str();
    }

    GLuint CompileStage(GLenum type, const std::string& path)
    {
        const std::string source = ReadFile(path);
        const char* sourcePtr = source.c_str();

        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &sourcePtr, nullptr);
        glCompileShader(shader);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLint length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            std::string log(length > 0 ? length : 1, '\0');
            glGetShaderInfoLog(shader, length, nullptr, log.data());
            glDeleteShader(shader);
            throw std::runtime_error("Shader compile failed (" + path + "):\n" + log);
        }
        return shader;
    }
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    GLuint vertex = CompileStage(GL_VERTEX_SHADER, vertexPath);

    GLuint fragment = 0;
    try
    {
        fragment = CompileStage(GL_FRAGMENT_SHADER, fragmentPath);
    }
    catch (...)
    {
        glDeleteShader(vertex);
        throw;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);

    glDetachShader(m_program, vertex);
    glDetachShader(m_program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint length = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length);
        std::string log(length > 0 ? length : 1, '\0');
        glGetProgramInfoLog(m_program, length, nullptr, log.data());
        glDeleteProgram(m_program);
        m_program = 0;
        throw std::runtime_error("Shader link failed (" + vertexPath + " + " + fragmentPath + "):\n" + log);
    }
}

Shader::~Shader()
{
    glDeleteProgram(m_program);
}

Shader::Shader(Shader&& other) noexcept
    : m_program(std::exchange(other.m_program, 0)),
    m_locations(std::move(other.m_locations))
{
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other)
    {
        glDeleteProgram(m_program);
        m_program = std::exchange(other.m_program, 0);
        m_locations = std::move(other.m_locations);
    }
    return *this;
}

void Shader::Bind() const
{
    glUseProgram(m_program);
}

void Shader::SetInt(const std::string& name, int value) const
{
    glProgramUniform1i(m_program, GetLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
    glProgramUniform1f(m_program, GetLocation(name), value);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const
{
    glProgramUniform3fv(m_program, GetLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const
{
    glProgramUniformMatrix4fv(m_program, GetLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

GLint Shader::GetLocation(const std::string& name) const
{
    auto it = m_locations.find(name);
    if (it != m_locations.end())
        return it->second;

    GLint location = glGetUniformLocation(m_program, name.c_str());
    if (location == -1)
        std::fprintf(stderr, "[Shader] Uniform '%s' not found (misspelled or optimised out)\n", name.c_str());

    m_locations.emplace(name, location);
    return location;
}
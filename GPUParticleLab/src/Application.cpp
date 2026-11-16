#include "../headers/Application.hpp"

//GLAD: Loads OpenGL function pointers
//GLFW: Handles window creation and inputs
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <stdexcept>

//Anonymous namespace is used to define functions 
//that are only used in this file. 
//This prevents name clashes with other files.
namespace
{
    //Called whenever GLFW encounters an error and prints it in
	//A human-readable format to the standard error stream.
    void GLFWErrorCallback(int code, const char* description)
    {
        std::fprintf(stderr, "[GLFW error %d] %s\n", code, description);
    }

	//Called automatically by GLFW whenever the window is resized.
	//It tells glviewport to resize the rendering area to match the new window size.
    void FramebufferSizeCallback(GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    //Called when detecting OpenGL errors
    //source: where message originated
	//type: type of message (error, warning, etc.)
	//id: unique identifier
	//severity: how severe the message is (high, medium, low)
	//message: the actual message from OpenGL
    void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei, const GLchar* message, const void*)
    {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

		//Converts the OpenGL severity enum to a human-readable string
        const char* sev = severity == GL_DEBUG_SEVERITY_HIGH ? "HIGH"
            : severity == GL_DEBUG_SEVERITY_MEDIUM ? "MEDIUM"
            : "LOW";
        std::fprintf(stderr, "[GL %s] id=%u source=0x%x type=0x%x\n    %s\n",
            sev, id, source, type, message);
    }
}

//Constructor for the Application class. 
//It initializes GLFW, creates a window, and sets up OpenGL context.
Application::Application(int width, int height, const char* title)
    : m_width(width), m_height(height)
{
    glfwSetErrorCallback(GLFWErrorCallback);

    if (!glfwInit())
        throw std::runtime_error("Failed to initialise GLFW");

	//Request an OpenGL 4.3 core profile context.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//Request a core profile context, which removes deprecated features.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
	//Request a debug context for OpenGL
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    //Creates an app window
	//nullptr, nullptr: no monitor, no shared context
    m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
    
    //Check whether window creation was successful
    if (!m_window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create window (OpenGL 4.3 core not supported?)");
    }

	//Make the OpenGL context of the window current on the calling thread
    glfwMakeContextCurrent(m_window);

	//Enable V-Sync to synchronize the frame rate
    //with the monitor's refresh rate
    glfwSwapInterval(1);

	//Register the framebuffer size callback to handle window resizing
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);

	//GLFW retrieves the address of the OpenGL function pointers, which are then loaded by GLAD.
	//MUST HAPPEN AFTER creating the OpenGL context and making it current.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        throw std::runtime_error("Failed to load OpenGL functions");
    }

    //Check whether our OpenGL context supports debugging
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, nullptr);
    }

    std::printf("OpenGL %s\nGLSL   %s\nGPU    %s\n",
        glGetString(GL_VERSION),
        glGetString(GL_SHADING_LANGUAGE_VERSION),
        glGetString(GL_RENDERER));

	//Sets the initial viewport size to match the window dimensions
    glViewport(0, 0, m_width, m_height);
}

//Destructor for the Application class.
Application::~Application()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

//Processes keyboard input
void Application::ProcessInput()
{
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

//Main loop of the application. 
//It continues until the window is closed.
void Application::Run()
{
    while (!glfwWindowShouldClose(m_window))
    {
		//process all pending events, such as keyboard and mouse input
        glfwPollEvents();
        ProcessInput();

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(m_window);
    }
}
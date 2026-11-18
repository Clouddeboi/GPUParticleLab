#pragma once

#include <glad/glad.h>

#include <memory>

#include "Camera.hpp"
#include "Shader.hpp"

struct GLFWwindow;

class Application
{
public:
	Application(int width, int height, const char* title);
	~Application();
	
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void Run();

private:
	struct GLFWLibrary
	{
		GLFWLibrary();
		~GLFWLibrary();
	};

	struct WindowDeleter
	{
		void operator()(GLFWwindow* window) const;
	};

	void ProcessInput();
	void OnFrameBufferResize(int width, int height);
	void OnScroll(float yOffset);

	GLFWLibrary m_glfw;

	//Unique pointer to manage the GLFW window
	//Automatically destroys the window when the Application object is destroyed
	std::unique_ptr<GLFWwindow, WindowDeleter> m_window;
	std::unique_ptr<Shader> m_shader;

	GLuint m_vao = 0;
	GLuint m_vbo = 0;

	Camera m_camera;
	double m_lastCursorX = 0.0;
	double m_lastCursorY = 0.0;
	bool m_orbiting = false;
};
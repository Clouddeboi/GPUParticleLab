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

//Registers our GLFW error callback and initializes the GLFW library.
Application::GLFWLibrary::GLFWLibrary()
{
	glfwSetErrorCallback(GLFWErrorCallback);
    if (!glfwInit())
        throw std::runtime_error("Failed to initialise GLFW");
}

//Terminates the GLFW library when the Application object is destroyed.
Application::GLFWLibrary::~GLFWLibrary()
{
    glfwTerminate();
}

//Custom deleter for GLFWwindow.
void Application::WindowDeleter::operator()(GLFWwindow* window) const
{
    glfwDestroyWindow(window);
}

//Constructor for the Application class. 
//It initializes GLFW, creates a window, and sets up OpenGL context.
//static_cast<float> converts the integer width and height to float for aspect ratio calculation.
Application::Application(int width, int height, const char* title)
    : m_camera(static_cast<float>(width) / static_cast<float>(height))
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
	m_window.reset(glfwCreateWindow(width, height, title, nullptr, nullptr));

    //Check whether window creation was successful
    if (!m_window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create window (OpenGL 4.3 core not supported?)");
    }

	//Make the OpenGL context of the window current on the calling thread
    glfwMakeContextCurrent(m_window.get());

	//Enable V-Sync to synchronize the frame rate
    //with the monitor's refresh rate
    glfwSwapInterval(1);

	//Register the framebuffer size callback to handle window resizing
    glfwSetFramebufferSizeCallback(m_window.get(), FramebufferSizeCallback);

	//GLFW retrieves the address of the OpenGL function pointers, which are then loaded by GLAD.
	//MUST HAPPEN AFTER creating the OpenGL context and making it current.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
		throw std::runtime_error("Failed to initialize GLAD");

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

	//Get the actual framebuffer size in pixels,
    //which may differ from the window size in screen coordinates
    int framebufferWidth = 0;
	int framebufferHeight = 0;

    glfwGetFramebufferSize(m_window.get(), &framebufferWidth, &framebufferHeight);
	
    //Apply the framebuffer dimensions to the camera and viewport
    OnFrameBufferResize(framebufferWidth, framebufferHeight);

	glEnable(GL_DEPTH_TEST);

	//Creates a shader program from vertex and fragment shader files.
	m_shader = std::make_unique<Shader>(
        "shaders/basic.vert",
        "shaders/basic.frag"
    );

    const float vertices[] = {
		//Triangle vertices (position and color)
        -0.5f, -0.5f, -0.3f,  1.0f, 0.6f, 0.1f,
         0.5f, -0.5f, -0.3f,  1.0f, 0.9f, 0.2f,
         0.0f,  0.5f, -0.3f,  0.9f, 0.3f, 0.7f,

		 //X axis line vertices (position and color)
         0.0f,  0.0f,  0.0f,  1.0f, 0.15f, 0.15f,
         1.5f,  0.0f,  0.0f,  1.0f, 0.15f, 0.15f,

		 //Y axis line vertices (position and color)
         0.0f,  0.0f,  0.0f,  0.15f, 1.0f, 0.15f,
         0.0f,  1.5f,  0.0f,  0.15f, 1.0f, 0.15f,

		 //Z axis line vertices (position and color)
         0.0f,  0.0f,  0.0f,  0.2f, 0.4f, 1.0f,
         0.0f,  0.0f,  1.5f,  0.2f, 0.4f, 1.0f,
    };

	//creates a Vertex Array Object (VAO) 
    //to store the vertex attribute configuration
    //and makes the VAO active
    glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	//Creates a Vertex Buffer Object (VBO)
    //to store vertex data in GPU memory
	//and uploads the vertex data to the GPU
    glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//There are 6 floats per vertex: 
    //3 for position and 3 for color.
	//The stride is the total size of one vertex in bytes.
	//Stride = 6 * sizeof(float)
	const GLsizei stride = 6 * sizeof(float);

	//Configures the vertex attribute pointers for position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);

	//Configures the vertex attribute pointers for color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

//Destructor for the Application class.
Application::~Application()
{
    //Context is still alive here
    //m_shader and m_window are destroyed after this body.
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

//Called whenever the window is resized.
void Application::OnFrameBufferResize(int width, int height)
{
	//prevents division by zero when calculating the aspect ratio
    if (width == 0 || height == 0)
        return;

	//Sets the OpenGL viewport to match the new window size
    glViewport(0, 0, width, height);

	//Updates the camera's aspect ratio to match the new window size
    m_camera.SetAspect(static_cast<float>(width) / static_cast<float>(height));
}

//Connects the mouse scroll input to the camera's zoom functionality.
void Application::OnScroll(float yOffset)
{
    m_camera.Zoom(yOffset);
}

//Processes keyboard input
void Application::ProcessInput()
{
	//Get the raw pointer to the GLFW window from the unique_ptr
    GLFWwindow* window = m_window.get();

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

	//Get the current cursor position in screen coordinates
    //from the top-left corner of the window
    double x = 0.0;
    double y = 0.0;
    glfwGetCursorPos(window, &x, &y);

	//If right mouse button is pressed and the user is currently orbiting
    const bool rightDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    if(rightDown && m_orbiting)
		m_camera.Orbit(static_cast<float>(x - m_lastCursorX), static_cast<float>(y - m_lastCursorY));

    m_orbiting = rightDown;
    m_lastCursorX = x;
    m_lastCursorY = y;
}

//Main loop of the application. 
//It continues until the window is closed.
void Application::Run()
{
    while (!glfwWindowShouldClose(m_window.get()))
    {
		//process all pending events, such as keyboard and mouse input
        glfwPollEvents();
        ProcessInput();

		//Clear the color and depth buffers to prepare for new frame rendering
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_shader->Bind();

        m_shader->SetMat4("uViewProjection", m_camera.GetViewProjectionMatrix());
        glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawArrays(GL_LINES, 3, 6);

        glfwSwapBuffers(m_window.get());
    }
}
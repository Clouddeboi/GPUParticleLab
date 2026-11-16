#pragma once

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
	void ProcessInput();

	GLFWwindow* m_window = nullptr;
	int m_width;
	int m_height;
};
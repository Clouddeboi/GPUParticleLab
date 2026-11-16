#include "../headers/Application.hpp"

#include <cstdio>
#include <exception>

int main()
{
    try
    {
        Application app(1600, 900, "GPU Particle Laboratory");
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::fprintf(stderr, "Fatal error: %s\n", e.what());
        return 1;
    }
    return 0;
}
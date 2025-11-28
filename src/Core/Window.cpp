#include "Window.h"
#include "Configuration.h"
#include "Vector2.hpp"
#include <stdexcept>

Window::Window(std::string &&winName_) :
    m_winName(std::move(winName_))
{
    auto resolution = ConfigurationManager::instance().m_settings["video"]["window_resolution"].readOrSet<Vector2<int>>({1920, 1080});

    m_window = SDL_CreateWindow(m_winName.c_str(), resolution.x, resolution.y, SDL_WINDOW_OPENGL);
	if (!m_window)
	{
		std::cout << "Window creation error: " << SDL_GetError() << std::endl;
        throw std::runtime_error("Cannot initialize window");
	}
}

Window::~Window()
{
    SDL_DestroyWindow(m_window);
}

Window& Window::operator=(Window &&rhs) noexcept
{
    m_window = rhs.m_window;
    m_winName = std::move(rhs.m_winName);

    rhs.m_window = nullptr;

    return *this;
}

Window::Window(Window &&rhs) noexcept : 
    m_window(rhs.m_window), 
    m_winName(std::move(rhs.m_winName))
{
    rhs.m_window = nullptr;
}

SDL_Window* Window::getWindow()
{
    return m_window;
}


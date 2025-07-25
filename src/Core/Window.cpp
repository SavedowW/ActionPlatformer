#include "Window.h"
#include "GameData.h"
#include "Configuration.h"
#include "JsonUtils.hpp"
#include <stdexcept>

Window::Window(const std::string &winName_) :
    m_winName(winName_)
{
    auto resolution = utils::tryClaimVector<int>(ConfigurationManager::instance().m_settings.read(), {"video", "window_resolution"}, {1920, 1080});

    m_window = SDL_CreateWindow(m_winName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, resolution.x, resolution.y, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (m_window == NULL)
	{
		std::cout << "Window creation error: " << SDL_GetError() << std::endl;
        throw std::runtime_error("Cannot initialize window");
	}
}

Window::~Window()
{
    SDL_DestroyWindow(m_window);
}

Window& Window::operator=(Window &&rhs)
{
    m_window = rhs.m_window;
    m_winName = rhs.m_winName;

    rhs.m_window = nullptr;

    return *this;
}

Window::Window(Window &&rhs)
{
    m_window = rhs.m_window;
    m_winName = rhs.m_winName;

    rhs.m_window = nullptr;
}

SDL_Window* Window::getWindow()
{
    return m_window;
}


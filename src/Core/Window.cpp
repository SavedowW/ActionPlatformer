#include "Window.h"
#include <stdexcept>

Window::Window(Vector2<int> resolution_, const std::string &winName_) :
    m_currentResolution(resolution_),
    m_winName(winName_)
{
    m_window = SDL_CreateWindow(m_winName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_currentResolution.x, m_currentResolution.y, SDL_WINDOW_SHOWN);
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
    m_currentResolution = rhs.m_currentResolution;

    rhs.m_window = nullptr;

    return *this;
}

Window::Window(Window &&rhs)
{
    m_window = rhs.m_window;
    m_winName = rhs.m_winName;
    m_currentResolution = rhs.m_currentResolution;

    rhs.m_window = nullptr;
}

SDL_Window* Window::getWindow()
{
    return m_window;
}

Vector2<int> Window::getCurrentResulution() const
{
    return m_currentResolution;
}
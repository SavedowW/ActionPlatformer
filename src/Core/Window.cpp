#include "Window.h"
#include "Configuration.h"
#include <stdexcept>

Window::Window(std::string &&winName_) :
    m_winName(std::move(winName_))
{
    m_resolution = ConfigurationManager::instance().m_settings["video"]["window_resolution"].readOrSet<Vector2<uint16_t>>({1920, 1080});

    m_window = SDL_CreateWindow(m_winName.c_str(), m_resolution.x, m_resolution.y, SDL_WINDOW_OPENGL);
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

Vector2<uint16_t> Window::getResolution() const noexcept
{
    return m_resolution;
}

SDL_Window* Window::getWindow() const noexcept
{
    return m_window;
}


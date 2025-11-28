#ifndef WINDOW_H_
#define WINDOW_H_

#include <SDL3/SDL.h>
#include <string>

class Window
{
public:
    Window(std::string &&winName_);
    ~Window();

    Window& operator=(const Window &rhs) = delete;
    Window(const Window &rhs) = delete;

    Window& operator=(Window &&rhs) noexcept;
    Window(Window &&rhs) noexcept;

    SDL_Window* getWindow();

private:
    SDL_Window* m_window = nullptr;
    std::string m_winName;
};

#endif
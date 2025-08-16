#ifndef WINDOW_H_
#define WINDOW_H_

#include <SDL3/SDL.h>
#include <string>
#include "Vector2.hpp"

class Window
{
public:
    Window(const std::string &winName_);
    ~Window();

    Window& operator=(const Window &rhs) = delete;
    Window(const Window &rhs) = delete;

    Window& operator=(Window &&rhs);
    Window(Window &&rhs);

    SDL_Window* getWindow();

private:
    SDL_Window* m_window = nullptr;
    std::string m_winName;
};

#endif
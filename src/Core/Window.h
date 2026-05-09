#pragma once
#include <SDL3/SDL.h>
#include <string>
#include "Vector2.hpp"

class Window
{
public:
    Window(std::string &&winName_);
    ~Window();

    Vector2<uint16_t> getResolution() const noexcept;

    SDL_Window* getWindow() const noexcept;

private:
    SDL_Window* m_window = nullptr;
    std::string m_winName;
    Vector2<uint16_t> m_resolution;
};

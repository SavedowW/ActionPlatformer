#include "Timer.h"
#include <iostream>
#include <SDL3/SDL.h>

void Timer::begin() noexcept
{
    m_timeBegin = SDL_GetTicksNS();
}

uint64_t Timer::iterate() noexcept
{
    const auto prev = m_timeBegin;
    m_timeBegin = SDL_GetTicksNS();
    return m_timeBegin - prev;
}

uint64_t Timer::getPassed() const noexcept
{
    return SDL_GetTicksNS() - m_timeBegin;
}

#include "Timer.h"
#include <iostream>
#include <SDL3/SDL.h>

void Timer::begin() noexcept
{
	m_timeBegin = SDL_GetTicksNS();
}

void Timer::profileDumpAndBegin(const std::string &msg_)
{
	auto newTicks = SDL_GetTicksNS();
	std::cout << msg_ << static_cast<float>(newTicks - m_timeBegin) / 1000000.0f << std::endl;
	m_timeBegin = newTicks;
}

uint64_t Timer::getPassed() const noexcept
{
	return SDL_GetTicksNS() - m_timeBegin;
}

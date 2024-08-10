#include "Timer.h"

Timer::Timer()
{
}

void Timer::begin()
{
	m_timeBegin = SDL_GetTicks();
}

uint32_t Timer::getPassedMS()
{
	return SDL_GetTicks() - m_timeBegin;
}

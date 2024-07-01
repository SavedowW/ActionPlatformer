#include "Timer.h"

Timer::Timer()
{
}

void Timer::begin()
{
	m_timeBegin = SDL_GetTicks();
}

Uint32 Timer::getPassedMS()
{
	return SDL_GetTicks() - m_timeBegin;
}

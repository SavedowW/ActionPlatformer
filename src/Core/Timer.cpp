#include "Timer.h"
#include <iostream>

Timer::Timer()
{
}

void Timer::begin()
{
	m_timeBegin = std::chrono::high_resolution_clock::now();
}

nanoseconds Timer::getPassedNano()
{
	return std::chrono::duration_cast<nanoseconds>(std::chrono::high_resolution_clock::now() - m_timeBegin);
}

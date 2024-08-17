#include "Timer.h"
#include <iostream>

Timer::Timer()
{
}

void Timer::begin()
{
	m_timeBegin = std::chrono::high_resolution_clock::now();
}

void Timer::profileDumpAndBegin(const std::string &msg_)
{
	std::cout << msg_ << getPassed<milliseconds>() << std::endl;
	begin();
}

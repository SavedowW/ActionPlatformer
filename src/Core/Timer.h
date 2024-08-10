#ifndef TIMER_H_
#define TIMER_H_
#include <SDL.h>

class Timer
{
public:
	Timer();
	void begin();
	uint32_t getPassedMS();

private:
	uint32_t m_timeBegin = 0;
};

#endif
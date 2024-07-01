#ifndef TIMER_H_
#define TIMER_H_
#include <SDL.h>

class Timer
{
public:
	Timer();
	void begin();
	Uint32 getPassedMS();

private:
	Uint32 m_timeBegin = 0;
};

#endif
#ifndef TIMER_H_
#define TIMER_H_
#include <chrono>

using std::chrono::nanoseconds;
using std::chrono::milliseconds;

class Timer
{
public:
	Timer();
	void begin();
	nanoseconds getPassedNano();

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_timeBegin;
};

#endif

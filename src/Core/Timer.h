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

	template<typename T>
	T getPassed() const
	{
		return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now() - m_timeBegin);
	}

	void profileDumpAndBegin(const std::string &msg_);

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_timeBegin;
};

#endif

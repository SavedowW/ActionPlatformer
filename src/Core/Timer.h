#ifndef TIMER_H_
#define TIMER_H_
#include <stdint.h>
#include <string>

class Timer
{
public:
	Timer() = default;

	void begin() noexcept;

	uint64_t getPassed() const noexcept;

	void profileDumpAndBegin(const std::string &msg_);

private:
	uint64_t m_timeBegin = 0;
};

#endif

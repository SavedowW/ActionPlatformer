#pragma once
#include <string>

// Works in nanoseconds
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

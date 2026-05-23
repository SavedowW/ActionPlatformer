#pragma once
#include <string>
#include <cstdint>

// Works in nanoseconds
class Timer
{
public:
    Timer() = default;

    void begin() noexcept;

    uint64_t getPassed() const noexcept;

    uint64_t iterate() noexcept;

private:
    uint64_t m_timeBegin = 0;
};

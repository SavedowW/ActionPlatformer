#pragma once
#include "Timer.h"

template<bool RESULT_IF_INACTIVE = false>
class ManualTimer
{
public:
    ManualTimer(const Time::NS &timeToWait_ = Time::NS{0}) noexcept;
    void begin(const Time::NS &timeToWait_) noexcept;
    void beginAt(const Time::NS &timeToWait_, float progress_) noexcept;
    bool update(const Time::NS &timePassed_) noexcept;
    bool isOver() const noexcept;
    bool isActive() const noexcept;
    float getProgressNormalized() const noexcept;
    void forceOver() noexcept;

private:
    Time::NS m_timeToWait;
    Time::NS m_timePassed;
};

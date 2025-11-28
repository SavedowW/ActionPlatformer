#include "ManualTimer.h"

template<bool RESULT_IF_INACTIVE>
ManualTimer<RESULT_IF_INACTIVE>::ManualTimer(const Time::NS &timeToWait_) noexcept
{
    begin(timeToWait_);
}

template<bool RESULT_IF_INACTIVE>
void ManualTimer<RESULT_IF_INACTIVE>::begin(const Time::NS &timeToWait_) noexcept
{
    m_timePassed = Time::NS{0};
    m_timeToWait = timeToWait_;
}

template <bool RESULT_IF_INACTIVE>
void ManualTimer<RESULT_IF_INACTIVE>::beginAt(const Time::NS &timeToWait_, float progress_) noexcept
{
    m_timePassed = Time::NS{static_cast<uint64_t>(static_cast<float>(timeToWait_.value()) * progress_)};
    m_timeToWait = timeToWait_;
}

template<bool RESULT_IF_INACTIVE>
bool ManualTimer<RESULT_IF_INACTIVE>::update(const Time::NS &timePassed_) noexcept
{
    if (m_timeToWait == Time::NS{0})
        return RESULT_IF_INACTIVE;

    if (m_timePassed < m_timeToWait)
        m_timePassed += timePassed_;

    return m_timePassed >= m_timeToWait;
}

template<bool RESULT_IF_INACTIVE>
bool ManualTimer<RESULT_IF_INACTIVE>::isOver() const noexcept
{
    if (m_timeToWait == Time::NS{0})
        return RESULT_IF_INACTIVE;

    return m_timePassed >= m_timeToWait;
}

template<bool RESULT_IF_INACTIVE>
bool ManualTimer<RESULT_IF_INACTIVE>::isActive() const noexcept
{
    if (m_timeToWait == Time::NS{0})
        return false;

    return m_timePassed < m_timeToWait;
}

template<bool RESULT_IF_INACTIVE>
float ManualTimer<RESULT_IF_INACTIVE>::getProgressNormalized() const noexcept
{
    if (m_timeToWait == Time::NS{0})
        return 0.0f;

    if (m_timePassed >= m_timeToWait)
        return 1.0f;

    return static_cast<float>(m_timePassed.value()) / static_cast<float>(m_timeToWait.value());
}

template <bool RESULT_IF_INACTIVE>
void ManualTimer<RESULT_IF_INACTIVE>::forceOver() noexcept
{
    m_timePassed = Time::NS{1};
    m_timeToWait = Time::NS{1};
}

template class ManualTimer<true>;
template class ManualTimer<false>;

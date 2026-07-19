#pragma once
#include "SlidingWindow.h"

template<typename T, size_t len>
void SlidingWindow<T, len>::push(const T &val_)
{
    auto &current = FixedQueue<T, len>::m_data[FixedQueue<T, len>::m_nextToFill];

    if (FixedQueue<T, len>::m_filled >= len)
    {
        m_sum -= current;
        m_sum += val_;
    }
    else
    {
        m_sum += val_;
        FixedQueue<T, len>::m_filled++;
    }

    current = val_;

    FixedQueue<T, len>::m_nextToFill = (FixedQueue<T, len>::m_nextToFill + 1) % len;
}

template<typename T, size_t len>
T SlidingWindow<T, len>::avg() const noexcept
{
    if (!FixedQueue<T, len>::m_filled)
        return 0;

    return m_sum / FixedQueue<T, len>::m_filled;
}


template<typename T, size_t len, uint8_t updatePeriod>
void SlidingWindowIterative<T, len, updatePeriod>::push(const T &val_)
{
    SlidingWindow<T, len>::push(val_);

    if (++m_iter % updatePeriod == 0)
        m_lastAvg = SlidingWindow<T, len>::m_sum / FixedQueue<T, len>::m_filled;
}

template<typename T, size_t len, uint8_t updatePeriod>
T SlidingWindowIterative<T, len, updatePeriod>::avg() const noexcept
{
    return m_lastAvg;
}

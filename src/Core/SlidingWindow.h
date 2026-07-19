#pragma once
#include "Utils.hpp"
#include "FixedQueue.hpp"

/*
    Each call of avg returns average of last `len` pushed values
*/
template<typename T, size_t len>
class SlidingWindow : public FixedQueue<T, len>
{
public:
    void push(const T &val_) override;

    virtual T avg() const noexcept;

protected:
    T m_sum = 0;
};


/*
    Each call of avg returns average of last `len` pushed values, but it's updated only on each `updatePeriod`th push
*/
template<typename T, size_t len, uint8_t updatePeriod>
class SlidingWindowIterative : public SlidingWindow<T, len>
{
public:
    void push(const T &val_) override;

    T avg() const noexcept override;

protected:
    uint8_t m_iter = 0;
    T m_lastAvg = 0;
};

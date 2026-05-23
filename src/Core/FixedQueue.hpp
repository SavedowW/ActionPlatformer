#pragma once
#include <stdexcept>
#include <iostream>
#include <array>

template <typename T, size_t len>
class FixedQueue
{
public:
    const T& operator[](size_t rhs_) const
    {
        if (rhs_ >= m_filled)
            throw std::runtime_error("Trying to access non-existing element");

        rhs_ += 1;

        if (m_nextToFill >= rhs_)
            return m_data[m_nextToFill - rhs_];

        return m_data[len - (rhs_ - m_nextToFill)];
    }

    virtual void push(const T &val_)
    {
        m_data[m_nextToFill] = val_;
        m_nextToFill = (m_nextToFill + 1) % len;
        m_filled = std::min(m_filled + 1, len);
    }

    constexpr size_t getLen() const noexcept
    {
        return len;
    }

    size_t getFilled() const noexcept
    {
        return m_filled;
    }

protected:
    std::array<T, len> m_data;
    size_t m_nextToFill = 0;
    size_t m_filled = 0;

};

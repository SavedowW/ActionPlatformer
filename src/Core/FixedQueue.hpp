#ifndef FIXED_QUEUE_H_
#define FIXED_QUEUE_H_

#include <stdexcept>
#include <iostream>

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
        else
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

namespace utils
{
    template <typename T, size_t len>
    inline void printQueue(const FixedQueue<T, len> &q_, char separator_ = ' ')
    {
        for (int i = 0; i < q_.getFilled(); ++i)
        {
            std::cout << q_[i] << separator_;
        }
        std::cout << "\n";
    }
}

#endif

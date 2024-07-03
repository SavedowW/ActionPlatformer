#ifndef FIXED_QUEUE_H_
#define FIXED_QUEUE_H_

#include <stdexcept>
#include <iostream>

template <typename T, size_t len>
class FixedQueue
{
public:
    FixedQueue() {};

    const T& operator[](int rhs_) const
    {
        if (rhs_ >= m_filled)
            throw std::runtime_error("Trying to access non-existing element");
    
        int lastFilled = m_nextToFill-1;
        if (lastFilled < 0)
            lastFilled = len - 1;
    
        int id = m_nextToFill - 1 - rhs_;
        if (id < 0)
            id = len + id;
    
        return m_data[id];
    }

    void push(const T &val_)
    {
        m_data[m_nextToFill] = val_;
        m_nextToFill = (m_nextToFill + 1) % len;
        m_filled = std::min(m_filled + 1, len);
    }

    constexpr int getLen() const
    {
        return len;
    }

    int getFilled() const
    {
        return m_filled;
    }

private:
    T m_data[len];
    int m_nextToFill = 0;
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
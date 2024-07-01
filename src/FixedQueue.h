#ifndef FIXED_QUEUE_H_
#define FIXED_QUEUE_H_

#include <stdexcept>
#include <iostream>

template <typename T, size_t len>
class FixedQueue
{
public:
    FixedQueue() {};

    const T& operator[](int rhs_) const;

    void push(const T &val_);

    constexpr int getLen() const;

    int getFilled() const;

private:
    T m_data[len];
    int m_nextToFill = 0;
    size_t m_filled = 0;

};

namespace utils
{
    template <typename T, size_t len>
    void printQueue(const FixedQueue<T, len> &q_, char separator_ = ' ')
    {
        for (int i = 0; i < q_.getFilled(); ++i)
        {
            std::cout << q_[i] << separator_;
        }
        std::cout << "\n";
    }
}

#endif
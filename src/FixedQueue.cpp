#include "FixedQueue.h"
#include "InputState.h"


template <typename T, size_t len>
const T& FixedQueue<T, len>::operator[](int rhs_) const
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

template <typename T, size_t len>
void FixedQueue<T, len>::push(const T &val_)
{
    m_data[m_nextToFill] = val_;
    m_nextToFill = (m_nextToFill + 1) % len;
    m_filled = std::min(m_filled + 1, len);
}

template <typename T, size_t len>
constexpr int FixedQueue<T, len>::getLen() const
{
    return len;
}

template <typename T, size_t len>
int FixedQueue<T, len>::getFilled() const
{
    return m_filled;
}

template FixedQueue<InputState, 30>;
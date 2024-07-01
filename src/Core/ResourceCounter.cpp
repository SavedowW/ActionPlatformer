#include "ResourceCounter.h"
#include <type_traits>

template <typename Resource_t>
ResourceCounter<Resource_t>::ResourceCounter(Resource_t max_, Resource_t consumed_) :
    m_max(max_),
    m_consumed(consumed_)
{
    static_assert(std::is_arithmetic_v<Resource_t>, "Type Resource_t must be an arithmetic type");
}

template <typename Resource_t>
bool ResourceCounter<Resource_t>::canConsume(Resource_t toConsume_) const
{
    return m_max >= m_consumed + toConsume_;
}

template <typename Resource_t>
bool ResourceCounter<Resource_t>::consume(Resource_t toConsume_)
{
    if (m_max >= m_consumed + toConsume_)
    {
        m_consumed += toConsume_;
        return true;
    }
    else
        return false;
}

template <typename Resource_t>
void ResourceCounter<Resource_t>::free()
{
    m_consumed = 0;
}

template ResourceCounter<int>;
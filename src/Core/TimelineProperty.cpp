#include "TimelineProperty.h"
#include "Vector2.h"
#include "StateMarker.hpp"

enum class CharacterState {};
enum class Enemy1States {};

template<typename T>
TimelineProperty<T>::TimelineProperty(std::vector<std::pair<uint32_t, T>> &&values_) :
    m_isEmpty(false)
{
    m_values = std::move(values_);

    std::ranges::sort(m_values,
    [](const std::pair<uint32_t, T> &lhs, const std::pair<uint32_t, T> &rhs) {
        return lhs.first < rhs.first;
    });

    if (m_values.size() == 0 || m_values.front().first != 0)
    {
        m_values.insert(m_values.begin(), std::make_pair<uint32_t, T>(0, std::move(T())));
    }
}

template<typename T>
TimelineProperty<T>::TimelineProperty(const TimelineProperty<T> &rhs)
{
    m_values = rhs.m_values;
    m_isEmpty = rhs.m_isEmpty;
}

template <typename T>
TimelineProperty<T> &TimelineProperty<T>::operator=(const TimelineProperty<T> &rhs)
{
    m_values = rhs.m_values;
    m_isEmpty = rhs.m_isEmpty;

    return *this;
}

template<typename T>
TimelineProperty<T>::TimelineProperty(T &&value_)
{
    m_values.push_back(std::make_pair<uint32_t, T>(0, std::move(value_)));
    m_isEmpty = false;
}

template<typename T>
TimelineProperty<T>::TimelineProperty(const T &value_)
{
    m_values.push_back(std::make_pair(0, value_));
    m_isEmpty = false;
}

template<typename T>
TimelineProperty<T>::TimelineProperty(TimelineProperty<T> &&rhs)
{
    m_values = std::move(rhs.m_values);
    m_isEmpty = rhs.m_isEmpty;
    rhs.m_isEmpty = true;
}

template<typename T>
TimelineProperty<T>::TimelineProperty()
{
    m_values.insert(m_values.begin(), std::make_pair<uint32_t, T>(0, std::move(T())));
    m_isEmpty = true;
}

template<typename T>
TimelineProperty<T>& TimelineProperty<T>::operator=(TimelineProperty<T> &&rhs)
{
    m_values = std::move(rhs.m_values);
    m_isEmpty = rhs.m_isEmpty;
    rhs.m_isEmpty = true;
    return *this;
}

template<typename T>
void TimelineProperty<T>::addPropertyValue(uint32_t timeMark_, T &&value_)
{
    auto it = std::upper_bound(m_values.begin(), m_values.end(), timeMark_,
        [](uint32_t timeMark, const auto& pair) {
            return timeMark <= pair.first;
        });

    if (it != m_values.end())
    {
        if (it->first == timeMark_)
            it->second = std::move(value_);
        else
            m_values.insert(it, std::make_pair(timeMark_, std::move(value_)));
    }
    else
    {
        m_values.push_back(std::make_pair(timeMark_, std::move(value_)));
    }

    m_isEmpty = false;
}

template<typename T>
const T &TimelineProperty<T>::operator[](uint32_t timeMark_) const
{
    auto it = std::lower_bound(m_values.rbegin(), m_values.rend(), timeMark_,
        [](const auto& pair, uint32_t timeMark) {
            return timeMark < pair.first;
        });
    if (it != m_values.rend())
        return it->second;
    return m_values.front().second;
}

template <typename T>
TimelinePropertyEditable<T>::TimelinePropertyEditable(std::vector<std::pair<uint32_t, T>> &&values_) :
    TimelineProperty<T>(std::move(values_))
{
}

template <typename T>
TimelinePropertyEditable<T>::TimelinePropertyEditable(T &&value_) :
    TimelineProperty<T>(std::move(value_))
{
}

template <typename T>
TimelinePropertyEditable<T>::TimelinePropertyEditable(const T &value_) :
    TimelineProperty<T>(value_)
{
}

template <typename T>
TimelinePropertyEditable<T>::TimelinePropertyEditable() :
    TimelineProperty<T>()
{
}

template <typename T>
int TimelinePropertyEditable<T>::getValuesCount() const
{
    return TimelineProperty<T>::m_values.size();
}

template <typename T>
std::pair<uint32_t, T> &TimelinePropertyEditable<T>::getValuePair(int id_)
{
    return TimelineProperty<T>::m_values[id_];
}

template <typename T>
void TimelinePropertyEditable<T>::setPairValue(int id_, T &&value_)
{
    TimelineProperty<T>::m_values[id_].second = std::move(value_);
}

template <typename T>
bool TimelinePropertyEditable<T>::deletePair(int id_)
{
    if (TimelineProperty<T>::m_values[id_].first == 0)
        return false;

    TimelineProperty<T>::m_values.erase(TimelineProperty<T>::m_values.begin() + id_);
    return true;
}

template <typename T>
void TimelinePropertyEditable<T>::clear()
{
    TimelineProperty<T>::m_values.clear();
    TimelineProperty<T>::m_isEmpty = true;
}

template<typename T>
bool TimelineProperty<T>::isEmpty() const
{
    return m_isEmpty;
}

template class TimelineProperty<bool>;
template class TimelineProperty<float>;
template class TimelineProperty<int>;
template class TimelineProperty<Vector2<float>>;
template class TimelineProperty<Vector2<int>>;
template class TimelineProperty<uint32_t>;
template class TimelineProperty<StateMarker>;

template class TimelinePropertyEditable<bool>;
template class TimelinePropertyEditable<float>;
template class TimelinePropertyEditable<int>;
template class TimelinePropertyEditable<Vector2<float>>;
template class TimelinePropertyEditable<Vector2<int>>;
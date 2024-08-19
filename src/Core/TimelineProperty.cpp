#include "TimelineProperty.hpp"
#include "Vector2.h"
#include "StateMarker.hpp"

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

template class TimelinePropertyEditable<bool>;
template class TimelinePropertyEditable<float>;
template class TimelinePropertyEditable<int>;
template class TimelinePropertyEditable<Vector2<float>>;
template class TimelinePropertyEditable<Vector2<int>>;
#ifndef TIMELINE_PROPERTY_H_
#define TIMELINE_PROPERTY_H_

#include <vector>
#include <algorithm>
#include <ranges>
#include <utility>
#include <cstdint>

template<typename T>
class TimelineProperty {
public:
    TimelineProperty(std::vector<std::pair<uint32_t, T>> &&values_);
    TimelineProperty(T &&value_);
    TimelineProperty(const T &value_);
    TimelineProperty();

    // std::pair that requires copy constructor is bad civilization
    TimelineProperty(const TimelineProperty<T> &rhs);
    TimelineProperty& operator=(const TimelineProperty<T> &rhs);
    TimelineProperty(TimelineProperty<T> &&rhs);
    TimelineProperty& operator=(TimelineProperty<T> &&rhs);

    void addPropertyValue(uint32_t timeMark_, T &&value_);

    bool isEmpty() const;

    const T &operator[](uint32_t timeMark_) const;

protected:
    std::vector<std::pair<uint32_t, T>> m_values;
    bool m_isEmpty;
};

template<typename T>
class TimelinePropertyEditable: public TimelineProperty<T>
{
public:
    TimelinePropertyEditable(std::vector<std::pair<uint32_t, T>> &&values_);
    TimelinePropertyEditable(T &&value_);
    TimelinePropertyEditable(const T &value_);
    TimelinePropertyEditable();

    int getValuesCount() const;
    std::pair<uint32_t, T> &getValuePair(int id_);
    void setPairValue(int id_, T &&value_);
    bool deletePair(int id_);
    void clear();
};

#endif
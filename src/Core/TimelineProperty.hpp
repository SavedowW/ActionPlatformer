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
    TimelineProperty(std::vector<std::pair<uint32_t, T>> &&values_) :
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
    
    TimelineProperty(T &&value_)
    {
        m_values.push_back(std::make_pair<uint32_t, T>(0, std::move(value_)));
        m_isEmpty = false;
    }

    TimelineProperty(const T &value_)
    {
        m_values.push_back(std::make_pair(0, value_));
        m_isEmpty = false;
    }

    TimelineProperty()
    {
        m_values.insert(m_values.begin(), std::make_pair<uint32_t, T>(0, std::move(T())));
        m_isEmpty = true;
    }

    TimelineProperty(const TimelineProperty<T> &rhs) = delete;
    TimelineProperty& operator=(const TimelineProperty<T> &rhs) = delete;

    TimelineProperty(TimelineProperty<T> &&rhs)
    {
        m_values = std::move(rhs.m_values);
        m_isEmpty = rhs.m_isEmpty;
        rhs.m_isEmpty = true;
    }

    TimelineProperty& operator=(TimelineProperty<T> &&rhs)
    {
        m_values = std::move(rhs.m_values);
        m_isEmpty = rhs.m_isEmpty;
        rhs.m_isEmpty = true;
        return *this;
    }

    void addPropertyValue(uint32_t timeMark_, T &&value_)
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

    bool isEmpty() const
    {
        return m_isEmpty;
    }

    const T &operator[](uint32_t timeMark_) const
    {
        auto it = std::lower_bound(m_values.rbegin(), m_values.rend(), timeMark_,
            [](const auto& pair, uint32_t timeMark) {
                return timeMark < pair.first;
            });
        if (it != m_values.rend())
            return it->second;
        return m_values.front().second;
    }

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
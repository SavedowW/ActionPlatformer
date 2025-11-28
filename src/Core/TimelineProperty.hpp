#ifndef TIMELINE_PROPERTY_H_
#define TIMELINE_PROPERTY_H_

#include <stdexcept>
#include <map>
#include <vector>

/*
    For each key K find values V1 where K1 is the greatest stored key with K1 < K
    So, for map:
        0: "zero"
        2: "two"
        5: "five"
    results are:
        0 - "zero"
        1 - "zero"
        2 - "two"
        3 - "two"
        4 - "two"
        5 - "five"
        6 - "five"
    For a move-only structs, use addPair
    Mostly designed to map properties to frames or time, has internal optimization
        for sequential access in both directions
        like 1 - 2 - 3 - ...
    Isn't exception-safe or thread-safe
*/
template<typename Tk, typename Tv>
class TimelineProperty {
public:
    // Requires copying due to std::map - inevitable because for nice syntax you need to build std::initializer_list
    TimelineProperty(const std::map<Tk, Tv> &values_) :
        m_isEmpty(false)
    {
        if (values_.empty())
            throw std::runtime_error("Trying to create a timeline property from an empty map");
        
        m_keys.reserve(values_.size());
        m_values.reserve(values_.size());

        for (const auto &el : values_)
        {
            m_keys.emplace_back(el.first);
            m_values.emplace_back(el.second);
        }

        m_last = 0;
        m_lastRequest = m_keys[m_last];
    }

    // Considered empty, but actually creates a default constructed element at 0
    TimelineProperty()
    {
        m_keys.emplace_back(0);
        m_values.emplace_back();
    }

    // Creates at 0, requires only move-constructor
    TimelineProperty(Tv &&rhs_) :
        m_isEmpty(false)
    {
        m_keys.emplace_back(0);
        m_values.emplace_back(std::move(rhs_));
    }

    // Requires copy-constructor
    TimelineProperty(const TimelineProperty<Tk, Tv> &rhs_) : 
        m_keys(rhs_.m_keys), 
        m_values(rhs_.m_values), 
        m_isEmpty(rhs_.m_isEmpty)
    {
        m_lastRequest = m_keys[m_last];
    }

    // Requires copy-constructor
    TimelineProperty& operator=(const TimelineProperty<Tk, Tv> &rhs_)
    {
        m_keys = rhs_.m_keys;
        m_values = rhs_.m_values;
        m_last = 0;
        m_lastRequest = m_keys[m_last];
        m_isEmpty = rhs_.m_isEmpty;

        return *this;
    }

    // Requires only move constructor
    TimelineProperty(TimelineProperty<Tk, Tv> &&rhs_) noexcept : 
        m_keys(std::move(rhs_.m_keys)), 
        m_values(std::move(rhs_.m_values)),
         m_isEmpty(rhs_.m_isEmpty)
    {
        m_lastRequest = m_keys[m_last];

        rhs_.m_last = 0;
        rhs_.m_lastRequest = Tk{0};
        rhs_.m_isEmpty = true;
    }

    // Requires only move constructor
    TimelineProperty& operator=(TimelineProperty<Tk, Tv> &&rhs_) noexcept
    {
        m_keys = std::move(rhs_.m_keys);
        m_values = std::move(rhs_.m_values);
        m_last = 0;
        m_lastRequest = m_keys[m_last];
        m_isEmpty = rhs_.m_isEmpty;

        rhs_.m_last = 0;
        rhs_.m_lastRequest = Tk{0};
        rhs_.m_isEmpty = true;

        return *this;
    }

    bool isEmpty() const noexcept
    {
        return m_isEmpty;
    }

    // Requires move constructor and assignment
    TimelineProperty &addPair(const Tk &timeMark_, Tv &&rhs_)
    {
        // TODO: what if timeMark_ < min?
        if (m_isEmpty)
        {
            m_keys = {timeMark_};
            m_values.clear();
            m_values.emplace_back(std::move(rhs_));

            m_last = 0;
            m_lastRequest = m_keys[m_last];
            m_isEmpty = false;
        }
        else
        {
            size_t id = binarySearch(timeMark_);
            if (m_keys[id] == timeMark_)
            {
                m_values[id] = std::move(rhs_);

                m_lastRequest = m_keys[m_last];
            }
            else
            {
                id++;

                m_keys.emplace(m_keys.begin() + id, timeMark_);
                m_values.emplace(m_values.begin() + id, std::move(rhs_));

                m_lastRequest = m_keys[m_last];
            }
        }

        return *this;
    }

    // Requires copy constructor and assignment
    TimelineProperty &addPair(Tk timeMark_, const Tv &rhs_)
    {
        // TODO: what if timeMark_ < min?
        if (m_isEmpty)
        {
            m_keys = {timeMark_};
            m_values.clear();
            m_values.emplace_back(rhs_);

            m_last = 0;
            m_lastRequest = m_keys[m_last];
            m_isEmpty = false;
        }
        else
        {
            size_t id = binarySearch(timeMark_);
            if (m_keys[id] == timeMark_)
            {
                m_values[id] = rhs_;

                m_lastRequest = m_keys[m_last];
            }
            else
            {
                id++;

                m_keys.emplace(m_keys.begin() + id, timeMark_);
                m_values.emplace(m_values.begin() + id, rhs_);

                m_lastRequest = m_keys[m_last];
            }
        }

        return *this;
    }

    const Tv &operator[](Tk timeMark_) const requires (!std::same_as<Tv, bool>)
    {
        iterateLastTo(timeMark_);
        return m_values.at(m_last);
    }

    Tv operator[](Tk timeMark_) const requires (std::same_as<Tv, bool>)
    {
        iterateLastTo(timeMark_);
        return m_values.at(m_last);
    }

protected:
    void iterateLastTo(Tk timeMark_) const
    {
        if (m_keys.empty() || m_values.empty())
            throw std::runtime_error("Trying to index a moved-from timeline");

        if (timeMark_ > m_lastRequest)
        {
            if (timeMark_ - m_lastRequest <= m_stepThreshold)
            {
                while (m_last != m_keys.size() - 1 && m_keys[m_last + 1] <= timeMark_)
                    m_last++;
            }
            else
                m_last = binarySearch(timeMark_);
        }
        else
        {
            if (m_lastRequest - timeMark_ <= m_stepThreshold)
            {
                while (m_last != 0 && m_keys[m_last] > timeMark_)
                    m_last--;
            }
            else
                m_last = binarySearch(timeMark_);
        }

        m_lastRequest = timeMark_;
    }

    size_t binarySearch(Tk timeMark_) const
    {
        const auto size = m_keys.size();
        if (timeMark_ >= m_keys[size - 1])
        {
            return size - 1;
        }

        if (timeMark_ < m_keys[0])
        {
            return 0;
        }

        size_t lBound = 0, rBound = size - 2;

        while (rBound - lBound > 1)
        {
            size_t idxBetween = (rBound + lBound) / 2;
            if (timeMark_ >= m_keys[idxBetween])
                lBound = idxBetween;
            else
                rBound = idxBetween - 1;
        }

        if (m_keys[rBound] <= timeMark_)
            return rBound;

        return lBound;
    }

    std::vector<Tk> m_keys;
    std::vector<Tv> m_values;
    mutable Tk m_lastRequest{0};
    mutable size_t m_last = 0;
    bool m_isEmpty = true;

    static constexpr Tk m_stepThreshold{5};
};

#endif

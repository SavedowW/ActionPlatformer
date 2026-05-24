#pragma once
#include <stdexcept>
#include <map>
#include <vector>
#include <format>

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
    Mostly designed to map properties to frames, has internal optimization
        for sequential access in both directions
        like 1 - 2 - 3 - ...
    Isn't exception-safe or thread-safe
*/
template<typename T>
class TimelineProperty {
public:
    // Requires copying due to std::map - inevitable because for nice syntax you need to build std::initializer_list
    TimelineProperty(const std::map<uint32_t, T> &values_)
    {
        if (values_.empty())
            throw std::runtime_error("Trying to create a timeline property from an empty map");

        m_data.reserve(values_.size());

        for (const auto &el : values_)
            m_data.emplace_back(el);

        m_lastRequest = m_data.end();
    }

    // Creates at 0, requires only move-constructor
    TimelineProperty(T &&rhs_) :
        m_data{{0, std::move(rhs_)}},
        m_lastRequest{m_data.begin()}
    {}

    TimelineProperty(const T &rhs_) :
        m_data{{0, rhs_}},
        m_lastRequest{m_data.begin()}
    {}

    TimelineProperty() = default;

    TimelineProperty(const TimelineProperty<T> &rhs_) = delete;
    TimelineProperty& operator=(const TimelineProperty<T> &rhs_) = delete;

    // Requires only move constructor
    TimelineProperty(TimelineProperty<T> &&rhs_) noexcept :
        m_data{std::move(rhs_.m_data)},
        m_lastRequest{m_data.begin()}
    {}

    // Requires only move constructor
    TimelineProperty& operator=(TimelineProperty<T> &&rhs_) noexcept
    {
        m_data = std::move(rhs_.m_data);
        m_lastRequest = m_data.begin();
        return *this;
    }

    bool isEmpty() const noexcept
    {
        return m_data.empty();
    }

    // Requires move constructor and assignment
    TimelineProperty &addPair(uint32_t timeMark_, T &&rhs_)
    {
        if (m_data.empty())
            m_data.emplace_back(timeMark_, std::move(rhs_));
        else
        {
            if (timeMark_ < m_data.front().first)
                m_data.emplace(m_data.begin(), timeMark_, std::move(rhs_));
            else
            {
                const auto it = binarySearch(timeMark_);
                if (it->first == timeMark_)
                    it->second = std::move(rhs_);
                else
                    m_data.emplace(it + 1, timeMark_, std::move(rhs_));
            }
        }

        m_lastRequest = m_data.begin();
        return *this;
    }

    // Requires copy constructor and assignment
    TimelineProperty &addPair(uint32_t timeMark_, const T &rhs_)
    {
        if (m_data.empty())
            m_data.emplace_back(timeMark_, rhs_);
        else
        {
            if (timeMark_ < m_data.front().first)
                m_data.emplace(m_data.begin(), timeMark_, rhs_);
            else
            {
                const auto it = binarySearch(timeMark_);
                if (it->first == timeMark_)
                    it->second = rhs_;
                else
                    m_data.emplace(it + 1, timeMark_, rhs_);
            }
        }

        m_lastRequest = m_data.begin();
        return *this;
    }

    const T &operator[](uint32_t timeMark_) const
    {
        iterateLastTo(timeMark_);
        return m_lastRequest->second;
    }

protected:
    void iterateLastTo(uint32_t timeMark_) const
    {
        if (m_data.empty())
            throw std::runtime_error("Trying to index an empty timeline");

        const int64_t diff = static_cast<int64_t>(timeMark_) - m_lastRequest->first;

        // Dirty, but only 2 conditions at most in probably the most important function in the project
        if (diff > 0)
        {
            if (diff <= 5)
            {
                while (m_lastRequest != m_data.end() - 1 && (m_lastRequest + 1)->first <= timeMark_)
                    ++m_lastRequest;
            }
            else
                m_lastRequest = binarySearch(timeMark_);
        }
        else if (diff < 0)
        {
            if (diff >= -5)
            {
                while (m_lastRequest != m_data.begin() && m_lastRequest->first > timeMark_)
                    m_lastRequest--;
            }
            else
                m_lastRequest = binarySearch(timeMark_);
        }
    }

    std::vector<std::pair<uint32_t, T>>::const_iterator binarySearch(const uint32_t timeMark_) const
    {
        const auto size = m_data.size();
        if (timeMark_ >= m_data[size - 1].first)
            return m_data.cbegin() + (size - 1);

        if (timeMark_ <= m_data[0].first)
            return m_data.cbegin();

        size_t lBound = 0, rBound = size - 2;

        while (rBound - lBound > 1)
        {
            size_t idxBetween = (rBound + lBound) / 2;
            if (timeMark_ >= m_data[idxBetween].first)
                lBound = idxBetween;
            else
                rBound = idxBetween - 1;
        }

        if (m_data[rBound].first <= timeMark_)
            return m_data.cbegin() + rBound;

        return m_data.cbegin() + lBound;
    }

    std::vector<std::pair<uint32_t, T>>::iterator binarySearch(const uint32_t timeMark_)
    {
        const auto size = m_data.size();
        if (timeMark_ >= m_data[size - 1].first)
            return m_data.begin() + (size - 1);

        if (timeMark_ <= m_data[0].first)
            return m_data.begin();

        size_t lBound = 0, rBound = size - 2;

        while (rBound - lBound > 1)
        {
            size_t idxBetween = (rBound + lBound) / 2;
            if (timeMark_ >= m_data[idxBetween].first)
                lBound = idxBetween;
            else
                rBound = idxBetween - 1;
        }

        if (m_data[rBound].first <= timeMark_)
            return m_data.begin() + rBound;

        return m_data.begin() + lBound;
    }

    std::vector<std::pair<uint32_t, T>> m_data;
    mutable std::vector<std::pair<uint32_t, T>>::const_iterator m_lastRequest = m_data.end();

    static constexpr int m_stepThreshold = 5;
};

/**
 *  NOTE: after several tests, it seems to be the fastest implementation for sequential access back-and-forth. Test results:
 * 
 *                                              filling    readRandom      readSequential
 *                                              
 * Int:
 * TimelinePropertyIndividualId:                24'000     43'217'700      22'889'100
 * TimelinePropertyIndividualIterator:          51'600     47'618'000      23'461'700
 * TimelinePropertyIndividualIteratorAlgorithm: 62'500     26'861'800      23'768'100
 * TimelinePropertySingleId:                    58'000     44'106'500      23'934'500
 * TimelinePropertySingleIterator:              45'300     45'921'700      22'647'900
 * TimelinePropertySingleIteratorAlgorithm:     55'800     23'253'100      22'194'300
 * TimelinePropertyMap:                         109'700    54'219'800      64'079'600
 * 
 * String:
 * TimelinePropertyIndividualId:                89'000     47'445'200      26'232'500
 * TimelinePropertyIndividualIterator:          99'000     44'769'700      27'240'900
 * TimelinePropertyIndividualIteratorAlgorithm: 104'900    26'214'200      26'353'500
 * TimelinePropertySingleId:                    111'700    45'465'600      26'853'500
 * TimelinePropertySingleIterator:              111'500    47'690'700      21'805'200
 * TimelinePropertySingleIteratorAlgorithm:     97'900     47'647'900      23'903'000
 * TimelinePropertyMap:                         159'600    56'887'000      73'036'900
 *
 * Here:
 *  - "Individual" - separate vectors for keys and values
 *  - "Single" - one vector of pairs for keys and values
 *  - "ID" - caching vector indices
 *  - "Iterator" - caching vector iterators (and using them for filling)
 *  - "Algorithm" - using upper_bound instead of custom implementation
 *  - TimelinePropertyMap used iterators and algo
 */

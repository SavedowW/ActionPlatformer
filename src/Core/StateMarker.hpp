#ifndef STATE_MARKER_H_
#define STATE_MARKER_H_

#include "StateCommon.h"
#include "StaticMapping.hpp"
#include <initializer_list>
#include <iostream>
#include <vector>
#include <chrono>

using StateHolder_t = uint64_t;
constexpr inline int STATE_HOLDER_SIZE = sizeof(StateHolder_t) * 8;

template<typename T>
concept TEnumUnsigned = TEnum<T> && std::unsigned_integral<std::underlying_type_t<T>>;

// Hold count_ bool values set to false by default
// For some reason works notably faster than just a regular std::vector<bool> with compiler optimization
// which should do the same thing under the hood
class StateMarker
{
public:
    StateMarker() = default;
    StateMarker(StateMarker &&rhs_) = default;
    StateMarker &operator=(StateMarker &&rhs_) = default;
    StateMarker(const StateMarker &rhs_) = default;
    StateMarker &operator=(const StateMarker &rhs_) = default;

    template<TEnumUnsigned ENUM_TYPE>
    constexpr StateMarker(const std::initializer_list<ENUM_TYPE> &trueFields_)  :
        m_stateMarks(static_cast<size_t>(getMaxValue<ENUM_TYPE>()) / STATE_HOLDER_SIZE + 1 , 0)
    {
        for (const auto &el: trueFields_)
        {
            auto casted = static_cast<int>(el);
            auto arrid = casted / STATE_HOLDER_SIZE;
            auto bitid = casted % STATE_HOLDER_SIZE;
            m_stateMarks[arrid] = m_stateMarks[arrid] | ((StateHolder_t)1 << bitid);
        }
    }

#if 0 // Requires resizing
    template<TEnumUnsigned ENUM_TYPE>
    constexpr void toggleMark(ENUM_TYPE id_)
    {
        auto casted = static_cast<int>(id_);
        auto arrid = casted / STATE_HOLDER_SIZE;
        auto bitid = casted % STATE_HOLDER_SIZE;
        m_stateMarks[arrid] = m_stateMarks[arrid] ^ ((StateHolder_t)1 << bitid);
    }
#endif

    template<typename T> requires std::unsigned_integral<T> || std::unsigned_integral<std::underlying_type_t<T>>
    constexpr bool operator[](const T &id_) const
    {
        const auto casted = static_cast<size_t>(id_);
        const auto arrid = casted / STATE_HOLDER_SIZE;

        if (arrid >= m_stateMarks.size())
            return false;

        const auto bitid = casted % STATE_HOLDER_SIZE;
        return ((m_stateMarks[arrid] >> bitid) & (StateHolder_t)1);
    }

private:
    std::vector<StateHolder_t> m_stateMarks;
};



#endif
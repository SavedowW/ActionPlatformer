#ifndef STATE_MARKER_H_
#define STATE_MARKER_H_

#include <iostream>
#include <array>
#include <chrono>

using StateHolder_t = uint64_t;
constexpr inline int STATE_HOLDER_SIZE = sizeof(StateHolder_t) * 8;

// Hold count_ bool values set to false by default
// For some reason works notably faster than just a regular std::vector<bool> with compiler optimization
// which should do the same thing under the hood
class StateMarker
{
public:
    StateMarker();
    StateMarker(int count_);
    StateMarker(const StateMarker &sm_);
    StateMarker &operator=(const StateMarker &sm_);
    StateMarker(StateMarker &&sm_);
    StateMarker(int count_, const std::vector<int> &trueFields_);
    void toggleMark(int id_);
    bool getMark(int id_) const;

    template<typename T>
    inline bool operator[](const T &id_) const
    {
        auto converted = static_cast<int>(id_);

        auto arrid = converted / STATE_HOLDER_SIZE;
        auto bitid = converted % STATE_HOLDER_SIZE;
        return ((m_stateMarks[arrid] >> bitid) & (StateHolder_t)1);
    }

    template<typename ENUM_NAME>
    inline StateMarker(ENUM_NAME lastElemP1_, const std::vector<ENUM_NAME> &trueFields_)  :
        m_stateMarks(static_cast<int>(lastElemP1_) / STATE_HOLDER_SIZE + 1 , 0)
    {
        for (const auto &el: trueFields_)
        {
            auto converted = static_cast<int>(el);
            auto arrid = converted / STATE_HOLDER_SIZE;
            auto bitid = converted % STATE_HOLDER_SIZE;
            m_stateMarks[arrid] = m_stateMarks[arrid] | ((StateHolder_t)1 << bitid);
        }
    }

private:
    std::vector<StateHolder_t> m_stateMarks;

};

#endif
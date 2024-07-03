#include "StateMarker.h"

StateMarker::StateMarker()
{
}

StateMarker::StateMarker(int count_) : m_stateMarks(count_ / STATE_HOLDER_SIZE + 1, 0)
{

}

StateMarker::StateMarker(const StateMarker &sm_)
{
    m_stateMarks = sm_.m_stateMarks;
}

StateMarker &StateMarker::operator=(const StateMarker &sm_)
{
    m_stateMarks = sm_.m_stateMarks;

    return *this;
}

StateMarker::StateMarker(StateMarker &&sm_)
{
    m_stateMarks = std::move(sm_.m_stateMarks);
}

StateMarker::StateMarker(int count_, const std::vector<int> &trueFields_) :
    m_stateMarks(count_ / STATE_HOLDER_SIZE + 1 , 0)
{
    for (const auto &el: trueFields_)
    {
        auto arrid = el / STATE_HOLDER_SIZE;
        auto bitid = el % STATE_HOLDER_SIZE;
        m_stateMarks[arrid] = m_stateMarks[arrid] | ((StateHolder_t)1 << bitid);
    }
}

void StateMarker::toggleMark(int id_)
{
    auto arrid = id_ / STATE_HOLDER_SIZE;
    auto bitid = id_ % STATE_HOLDER_SIZE;
    m_stateMarks[arrid] = m_stateMarks[arrid] ^ ((StateHolder_t)1 << bitid);
}

bool StateMarker::getMark(int id_) const
{
    auto arrid = id_ / STATE_HOLDER_SIZE;
    auto bitid = id_ % STATE_HOLDER_SIZE;
    return ((m_stateMarks[arrid] >> bitid) & (StateHolder_t)1);
}

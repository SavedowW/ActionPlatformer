#include "Profile.h"
#include "Utils.hpp"
#include <iostream>

TimeStatistic &TimeStatistic::operator+=(const uint64_t &rhs_) noexcept
{
    m_sum += rhs_;
    m_cnt++;
    return *this;
}

uint64_t TimeStatistic::avg() const noexcept
{
    if (m_cnt == 0)
        return 0;
    return m_sum / m_cnt;
}

uint64_t TimeStatistic::sum() const noexcept
{
    return m_sum;
}

int TimeStatistic::count() const noexcept
{
    return m_cnt;
}

void TimeStatistic::reset() noexcept
{
    m_sum = 0;
    m_cnt = 0;
}

Profiler &Profiler::instance() noexcept
{
    static Profiler profiler;
    return profiler;
}

void Profiler::addRecord(const size_t &id_, uint64_t duration_) noexcept
{
    m_calls[id_].m_timeStat += duration_;
}

size_t Profiler::addName(const char *location_, const std::string &name_, int line_) noexcept
{
    m_calls.emplace_back(name_, location_, line_);
    if (name_.size() > m_longestFuncName)
        m_longestFuncName = name_.size();
    return m_calls.size() - 1;
}

void Profiler::cleanFrame() noexcept
{
#ifdef DUMP_PROFILE
    for (auto &el : m_calls)
        el.m_timeStat.reset();
#endif
}


void Profiler::dump() const noexcept
{
#ifdef DUMP_PROFILE_CONSOLE
    std::string accum = utils::padToRight(m_longestFuncName, "function") 
                + ' ' + utils::padToRight(10, "avg, ms")
                + ' ' + utils::padToRight(10, "sum, ms")
                + ' ' + utils::padToRight(5, "count") + '\n';
    for (const auto &el : m_calls)
    {
        accum +=        utils::padToRight(m_longestFuncName, el.m_funcName);
        accum += " "s + utils::padToRight(10, std::to_string(static_cast<float>(el.m_timeStat.avg()) / 1'000'000.0f));
        accum += " "s + utils::padToRight(10, std::to_string(static_cast<float>(el.m_timeStat.sum()) / 1'000'000.0f));
        accum += " "s + utils::padToRight(5, std::to_string(el.m_timeStat.count())) + '\n';
    }
    std::cout << accum << std::endl;
#endif
}

ProfileTimer::ProfileTimer(const size_t &id_) noexcept :
    m_id(id_)
{
    begin();
    m_stopped = false;
}

void ProfileTimer::stop() noexcept
{
    const auto passedNS = getPassedNS();
    Profiler::instance().addRecord(m_id, passedNS.value());
    m_stopped = true;
}

ProfileTimer::~ProfileTimer()
{
    if (!m_stopped)
        stop();
}

size_t registerProfilePlace(const char *file_, const std::string &functionName_, int line_) noexcept
{
    return Profiler::instance().addName(file_, functionName_, line_);
}

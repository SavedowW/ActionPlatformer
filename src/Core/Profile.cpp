#include "Profile.h"
#include <iostream>

Profiler &Profiler::instance()
{
    static Profiler profiler;
    return profiler;
}

void Profiler::addRecord(const std::string &identifier_, nanoseconds duration_)
{
    m_calls[identifier_] += duration_;
}

void Profiler::addName(const char *identifier_, const std::string &name_)
{
    m_names[std::string(identifier_) + name_] = name_;
}

void Profiler::cleanFrame()
{
    m_calls.clear();
}


void Profiler::dump() const
{
    for (auto &el : m_names)
    {
        auto found = m_calls.find(el.first);
        if (found != m_calls.end())
            std::cout << el.second << ": avg = " << found->second.avg().count() / 1000000.0f << "ms, sum = " << found->second.sum().count() / 1000000.0f << "ms, calls = " << found->second.count() << std::endl;
        else
            std::cout << el.second << ": avg = " << 0 << ", sum = " << 0 << ", calls = " << 0 << std::endl;
    }
}

ProfileTimer::ProfileTimer(const char *identifier_, const std::string &functionName_) :
    m_identifier(std::string(identifier_) + functionName_)
{
    begin();
    m_stopped = false;
}

void ProfileTimer::stop()
{
    auto passed = getPassed<nanoseconds>();
    Profiler::instance().addRecord(m_identifier, passed);
    m_stopped = true;
}

ProfileTimer::~ProfileTimer()
{
    if (!m_stopped)
        stop();
}

SingleProfileRegister::SingleProfileRegister(const char *identifier_, const std::string &name_)
{
    Profiler::instance().addName(identifier_, name_);
}

#ifndef PROFILE_H_
#define PROFILE_H_
#include "Timer.h"
#include "Logger.h"
#include <unordered_map>

//#define DUMP_PROFILE_CONSOLE
//#define DUMP_PROFILE_UI // TODO:

#if defined(DUMP_PROFILE_CONSOLE) || defined(DUMP_PROFILE_UI)
    #define DUMP_PROFILE
#endif

class TimeStatistic
{
public:
    TimeStatistic() = default;

    TimeStatistic &operator+=(const uint64_t &rhs_)
    {
        m_sum += rhs_;
        m_cnt++;
        return *this;
    }

    uint64_t avg() const
    {
        return m_sum / m_cnt;
    }

    uint64_t sum() const
    {
        return m_sum;
    }

    int count() const
    {
        return m_cnt;
    }

    inline bool isSet() const
    {
        return m_cnt > 0;
    }

private:
    uint64_t m_sum = 0;
    uint64_t m_cnt = 0;

};

class ProfileTimer : public Timer
{
public:
    ProfileTimer(const char *identifier_, const std::string &functionName_);
    void stop();
    ~ProfileTimer();

private:
    bool m_stopped = false;
    std::string m_identifier;
};

class Profiler
{
public:
    static Profiler &instance();
    void addRecord(const std::string &identifier_, uint64_t duration_);
    void addName(const char *identifier_, const std::string &name_);
    void cleanFrame();
    void dump() const;

private:
    std::unordered_map<std::string, TimeStatistic> m_calls;
    std::unordered_map<std::string, std::string> m_names;
};

class SingleProfileRegister
{
public:
    SingleProfileRegister(const char *identifier_, const std::string &name_);
};

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define FILE_LINE_STRING TOSTRING(__FILE__) "_" TOSTRING(__LINE__)
#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define VARNAME(name) CONCATENATE(name, __LINE__)##_SRw453ytueh

#ifdef DUMP_PROFILE

#define PROFILE_FUNCTION static SingleProfileRegister VARNAME(NAMEREG)(FILE_LINE_STRING, utils::prettifyFunction(__FUNCSIG__)); ProfileTimer VARNAME(PROFILETMR)(FILE_LINE_STRING, utils::prettifyFunction(__FUNCSIG__));
#define PROFILE_SCOPE(scopename) static SingleProfileRegister VARNAME(NAMEREG)(FILE_LINE_STRING, #scopename); ProfileTimer VARNAME(PROFILETMR)(FILE_LINE_STRING, utils::prettifyFunction(__FUNCSIG__));

#else

#define PROFILE_FUNCTION
#define PROFILE_SCOPE(scopename)

#endif

#endif
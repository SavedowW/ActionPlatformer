#ifndef PROFILE_H_
#define PROFILE_H_
#include "Timer.h"
#include "Logger.h" // IWYU pragma: keep
#include <vector>

//#define DUMP_PROFILE_CONSOLE
//#define DUMP_PROFILE_UI // TODO:

#if defined(DUMP_PROFILE_CONSOLE) || defined(DUMP_PROFILE_UI)
    #define DUMP_PROFILE
#endif

class TimeStatistic
{
public:
    TimeStatistic &operator+=(const uint64_t &rhs_) noexcept;
    uint64_t avg() const noexcept;
    uint64_t sum() const noexcept;
    int count() const noexcept;
    void reset() noexcept;

private:
    uint64_t m_sum = 0;
    uint64_t m_cnt = 0;
};

struct CallData
{
    [[maybe_unused]]
    const std::string m_funcName;

    [[maybe_unused]]
    const std::string m_location;

    [[maybe_unused]]
    const unsigned int m_line;

    TimeStatistic m_timeStat;
};

class ProfileTimer : public Timer
{
public:
    ProfileTimer(const size_t &id_) noexcept;
    void stop() noexcept;
    ~ProfileTimer();

private:
    bool m_stopped = false;
    const size_t &m_id;
};

class Profiler
{
public:
    static Profiler &instance() noexcept;
    void addRecord(const size_t &id_, uint64_t duration_) noexcept;
    size_t addName(const char *location_, const std::string &name_, int line_) noexcept;
    void cleanFrame() noexcept;
    void dump() const noexcept;

private:
    std::vector<CallData> m_calls;
    size_t m_longestFuncName = 0;
};

size_t registerProfilePlace(const char *file_, const std::string &functionName_, int line_) noexcept;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define FILENAME __FILE__
#define LINENUM __LINE__
#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define VARNAME(name) CONCATENATE(CONCATENATE(name, __LINE__), _SRw453ytueh)

#ifdef __clang__
#define FUNCNAME __PRETTY_FUNCTION__
#else
#define FUNCNAME __FUNCSIG__
#endif


#ifdef DUMP_PROFILE

#define PROFILE_FUNCTION static auto VARNAME(NAMEREG) = registerProfilePlace(FILENAME, utils::prettifyFunction(FUNCNAME), LINENUM); \
        ProfileTimer VARNAME(PROFILETMR)(VARNAME(NAMEREG));

#else

#define PROFILE_FUNCTION

#endif

#endif
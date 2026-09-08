#pragma once
#include "StaticMapping.hpp"
#include <print>
#include <chrono>
#include <string_view>

namespace utils
{
    void localtimeImpl(tm *tm_, const time_t *time_);
    std::string prettifyFunction(const std::string &functionName_);
    std::string cutBoundingSpaces(const std::string &functionName_);
}

namespace Logger
{
    enum class Level : uint8_t
    {
        TRACE = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    };

    template<typename... Args>
    void logImpl(const Level &level_, const std::string_view &funcName_, const std::string_view &text_)
    {
        const auto nowts = std::chrono::system_clock::now();
        const auto now = std::chrono::system_clock::to_time_t(nowts);

        const auto mks = std::chrono::duration_cast<std::chrono::microseconds>(
            nowts.time_since_epoch()
            ).count() % 1'000'000;

        tm now_t;
        utils::localtimeImpl(&now_t, &now);

        std::print("{}.{:0>2}.{:0>2} {:0>2}:{:0>2}:{:0>2}.{:0>6} {} {}: {}\n", now_t.tm_year + 1900, now_t.tm_mon + 1, now_t.tm_mday, now_t.tm_hour, now_t.tm_min, now_t.tm_sec, mks, serialize(level_), funcName_, text_);
    }

    template<typename... Args>
    void logImpl(const Level &level_, const std::string_view &funcName_, const std::format_string<Args...> &text_, Args&&... args_) requires (sizeof...(Args) > 0)
    {
        const auto nowts = std::chrono::system_clock::now();
        const auto now = std::chrono::system_clock::to_time_t(nowts);

        const auto mks = std::chrono::duration_cast<std::chrono::microseconds>(
            nowts.time_since_epoch()
            ).count() % 1'000'000;

        tm now_t;
        utils::localtimeImpl(&now_t, &now);

        std::print("{}.{:0>2}.{:0>2} {:0>2}:{:0>2}:{:0>2}.{:0>6} {} {}: {}\n", now_t.tm_year + 1900, now_t.tm_mon + 1, now_t.tm_mday, now_t.tm_hour, now_t.tm_min, now_t.tm_sec, mks, serialize(level_), funcName_, std::format(text_, std::forward<Args>(args_)...));
    }
}

SERIALIZE_ENUM(Logger::Level, {
    ENUM_INIT(Logger::Level, TRACE, "TRC"),
    ENUM_INIT(Logger::Level, INFO, "INF"),
    ENUM_INIT(Logger::Level, WARNING, "WRN"),
    ENUM_INIT(Logger::Level, ERROR, "ERR")
});

#define LOG_IMPL(LVL, ...) \
try { \
    if (LVL >= Logger::Level::TRACE) \
    { \
        Logger::logImpl(LVL, __func__, __VA_ARGS__); \
    } \
} catch (const std::exception &ex_) { \
    std::print("Error while printing log at {}:{}: {}", __FILE__, __LINE__, ex_.what()); \
}

#define LOG_TRACE(...) LOG_IMPL(Logger::Level::TRACE, __VA_ARGS__)
#define LOG_INFO(...) LOG_IMPL(Logger::Level::INFO, __VA_ARGS__)
#define LOG_WARNING(...) LOG_IMPL(Logger::Level::WARNING, __VA_ARGS__)
#define LOG_ERROR(...) LOG_IMPL(Logger::Level::ERROR, __VA_ARGS__)

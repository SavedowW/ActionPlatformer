#pragma once
#include <string>
#include "StaticMapping.hpp"
#include <format>

struct ComponentName
{
    std::string name;
};

namespace utils
{
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
    void logImpl(const Level &level_, const std::string_view &funcName_, const std::string_view &text_);

    template<typename... Args>
    void logImpl(const Level &level_, const std::string_view &funcName_, const std::format_string<Args...> &text_, Args&&... args_) requires (sizeof...(Args) > 0);

    template<typename... Args>
    void logImpl(const Level &level_, const std::string_view &funcName_, const ComponentName &source_, const std::format_string<Args...> &text_, Args&&... args_);
}

SERIALIZE_ENUM(Logger::Level, {
    ENUM_INIT(Logger::Level, TRACE, "TRC"),
    ENUM_INIT(Logger::Level, INFO, "INF"),
    ENUM_INIT(Logger::Level, WARNING, "WRN"),
    ENUM_INIT(Logger::Level, ERROR, "ERR")
})

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


// TODO:

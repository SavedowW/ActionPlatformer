#pragma once
#include "Logger.h"
#include <print>
#include <chrono>

namespace Logger
{

    template<typename... Args>
    void logImpl(const Level &level_, const std::string_view &funcName_, const std::string_view &text_)
    {
        const auto nowts = std::chrono::system_clock::now();
        const auto now = std::chrono::system_clock::to_time_t(nowts);

        const auto mks = std::chrono::duration_cast<std::chrono::microseconds>(
            nowts.time_since_epoch()
            ).count() % 1'000'000;

        tm now_t;
        localtime_s(&now_t, &now);

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
        localtime_s(&now_t, &now);

        std::print("{}.{:0>2}.{:0>2} {:0>2}:{:0>2}:{:0>2}.{:0>6} {} {}: {}\n", now_t.tm_year + 1900, now_t.tm_mon + 1, now_t.tm_mday, now_t.tm_hour, now_t.tm_min, now_t.tm_sec, mks, serialize(level_), funcName_, std::format(text_, std::forward<Args>(args_)...));
    }

    template<typename... Args>
    void logImpl(const Level &level_, const std::string_view &funcName_, const ComponentName &source_, const std::format_string<Args...> &text_, Args&&... args_)
    {
        const auto nowts = std::chrono::system_clock::now();
        const auto now = std::chrono::system_clock::to_time_t(nowts);

        const auto mks = std::chrono::duration_cast<std::chrono::microseconds>(
            nowts.time_since_epoch()
            ).count() % 1'000'000;

        tm now_t;
        localtime_s(&now_t, &now);

        std::print("{}.{:0>2}.{:0>2} {:0>2}:{:0>2}:{:0>2}.{:0>6} {} [{}] {}: {}\n", now_t.tm_year + 1900, now_t.tm_mon + 1, now_t.tm_mday, now_t.tm_hour, now_t.tm_min, now_t.tm_sec, mks, serialize(level_), source_.name, funcName_, std::format(text_, std::forward<Args>(args_)...));
    }

}


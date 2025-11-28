#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <iostream>

namespace Time
{
    // Nanoseconds
    class NS
    {
    public:
        constexpr NS() noexcept = default;
        constexpr NS(const NS&) noexcept = default;
        constexpr NS(NS&&) noexcept = default;
        constexpr NS &operator=(const NS&) noexcept = default;
        constexpr NS &operator=(NS&&) noexcept = default;

        explicit constexpr NS(const uint64_t &time_) noexcept :
            m_time{time_}
        {}

        constexpr bool operator<(const NS &rhs_) const noexcept  { return this->m_time < rhs_.m_time; }
        constexpr bool operator<=(const NS &rhs_) const noexcept  { return this->m_time <= rhs_.m_time; }
        constexpr bool operator==(const NS &rhs_) const noexcept  { return this->m_time == rhs_.m_time; }
        constexpr bool operator>=(const NS &rhs_) const noexcept  { return this->m_time >= rhs_.m_time; }
        constexpr bool operator>(const NS &rhs_) const noexcept  { return this->m_time > rhs_.m_time; }

        constexpr operator std::string() const { return std::to_string(m_time) + "ns"; };
        constexpr NS operator+(const NS &rhs_) const noexcept { return NS{m_time + rhs_.m_time}; }
        constexpr NS operator-(const NS &rhs_) const noexcept { return NS{m_time - rhs_.m_time}; }

        constexpr NS &operator-=(const NS &rhs_) noexcept
        {
            assert(m_time >= rhs_.m_time);
            m_time -= rhs_.m_time;
            return *this;
        }

        constexpr NS &operator+=(const NS &rhs_) noexcept
        {
            m_time += rhs_.m_time;
            return *this;
        }

        constexpr NS operator%(const NS &rhs_) const noexcept { return NS{m_time % rhs_.m_time}; }

        template<typename T>
        constexpr NS operator*(const T &rhs_) const noexcept { return NS{static_cast<uint64_t>(m_time * rhs_)}; }

        constexpr uint64_t value() const noexcept { return m_time; }

    private:
        friend std::ostream &operator<<(std::ostream &lhs_, const NS &rhs_);

        uint64_t m_time = 0;
    };

    NS deserialize(const std::string &s_);
    NS fromFrames(uint64_t frame_);
}

// Works in nanoseconds
class Timer
{
public:
    Timer() noexcept = default;

    void begin() noexcept;

    Time::NS getPassedNS() const noexcept;

    // return passed time and immediately begin again
    Time::NS iterateNS() noexcept;

private:
    Time::NS m_timeBegin{0};
};

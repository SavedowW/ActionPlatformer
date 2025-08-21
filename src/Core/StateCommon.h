#ifndef STATE_COMMON_H_
#define STATE_COMMON_H_
#include <cstdint>
#include <type_traits>

using CharState = uint8_t;

// Wrapper for enums used as flags
template<typename T> requires std::is_enum_v<T>
class Flag
{
using UT = std::underlying_type_t<T>;

public:
    constexpr Flag(const T &flag_) noexcept :
        m_flag(flag_)
    {}

    constexpr operator bool() const noexcept
    {
        return static_cast<UT>(m_flag);
    }

    constexpr operator T() const noexcept
    {
        return m_flag;
    }

    constexpr auto operator|(const Flag<T> &cmp_) const noexcept
    {
        return static_cast<T>(static_cast<UT>(m_flag) | static_cast<UT>(cmp_.m_flag));
    }

    constexpr auto operator|(const T &cmp_) const noexcept
    {
        return static_cast<T>(static_cast<UT>(m_flag) | static_cast<UT>(cmp_));
    }

    constexpr auto operator&(const Flag<T> &cmp_) const noexcept
    {
        return static_cast<T>(static_cast<UT>(m_flag) & static_cast<UT>(cmp_.m_flag));
    }

    constexpr auto operator&(const T &cmp_) const noexcept
    {
         return static_cast<T>(static_cast<UT>(m_flag) & static_cast<UT>(cmp_));
    }

    constexpr auto operator|=(const Flag<T> &cmp_) noexcept
    {
        m_flag = static_cast<T>(static_cast<UT>(m_flag) | static_cast<UT>(cmp_.m_flag));
        return *this;
    }

    constexpr auto &operator|=(const T &cmp_) noexcept
    {
        m_flag = static_cast<T>(static_cast<UT>(m_flag) | static_cast<UT>(cmp_));
        return *this;
    }

private:
    T m_flag;
};

#endif

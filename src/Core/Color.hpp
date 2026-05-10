#pragma once
#include <type_traits>
#include <cstdint>

template <typename T>
concept Integral = std::is_integral_v<T>;

class Color
{
public:
    template<Integral T>
    constexpr Color(T r_, T g_, T b_, T a_)  :
        m_r{static_cast<float>(r_) / 255.0f},
        m_g{static_cast<float>(g_) / 255.0f},
        m_b{static_cast<float>(b_) / 255.0f},
        m_a{static_cast<float>(a_) / 255.0f}
    {}

    constexpr Color(float r_, float g_, float b_, float a_) :
        m_r{r_ / 255.0f},
        m_g{g_ / 255.0f},
        m_b{b_ / 255.0f},
        m_a{a_ / 255.0f}
    {}

    constexpr float r() const noexcept { return m_r; }
    constexpr float g() const noexcept { return m_g; }
    constexpr float b() const noexcept { return m_b; }
    constexpr float a() const noexcept { return m_a; }

    constexpr uint8_t ir() const noexcept { return static_cast<uint8_t>(m_r * 255.0f); }
    constexpr uint8_t ig() const noexcept { return static_cast<uint8_t>(m_g * 255.0f); }
    constexpr uint8_t ib() const noexcept { return static_cast<uint8_t>(m_b * 255.0f); }
    constexpr uint8_t ia() const noexcept { return static_cast<uint8_t>(m_a * 255.0f); }

private:
    const float m_r = 0.0f;
    const float m_g = 0.0f;
    const float m_b = 0.0f;
    const float m_a = 0.0f;
};

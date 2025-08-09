#ifndef VECTOR2_H_
#define VECTOR2_H_
#include "Utils.h"
#include "EnumMapping.hpp"
#include <cmath>
#include <iostream>
#include <type_traits>
#include <map>
#include <sstream>
#include <cassert>

enum class ORIENTATION : int8_t
{
    RIGHT = 1,
    LEFT = -1,
    UNSPECIFIED = 0
};

SERIALIZE_ENUM(ORIENTATION, {
    ENUM_AUTO(ORIENTATION, RIGHT),
    ENUM_AUTO(ORIENTATION, LEFT),
    ENUM_AUTO(ORIENTATION, UNSPECIFIED)
})

template<Numeric T>
inline constexpr ORIENTATION ValueToOrientation(const T &rhs_) noexcept
{
    return static_cast<ORIENTATION>((rhs_ > 0) - (rhs_ < 0));
}

template<Numeric T>
inline constexpr int ValueToOrientationInt(const T &rhs_) noexcept
{
    return static_cast<int>((rhs_ > 0) - (rhs_ < 0));
}


template <Numeric T>
struct Vector2
{
    T x, y;
    constexpr inline Vector2(T nx = 0, T ny = 0) noexcept
    {
        x = nx;
        y = ny;
    }

    template <Numeric TR>
    constexpr inline Vector2(const Vector2<TR> &rhs) noexcept
    {
        x = static_cast<T>(rhs.x);
        y = static_cast<T>(rhs.y);
    }

    constexpr inline Vector2(ORIENTATION orient_) noexcept
    {
        x = static_cast<T>(orient_);
        y = 0;
    }

    template <Numeric TR>
    constexpr inline Vector2<T> &operator=(const Vector2<TR> &rhs) noexcept
    {
        x = static_cast<T>(rhs.x);
        y = static_cast<T>(rhs.y);
        return *this;
    }

    template <Numeric TR>
    constexpr inline bool operator==(const Vector2<TR> &rhs) const noexcept
    {
        return (x == rhs.x && y == rhs.y);
    }

    constexpr inline bool operator==(const ORIENTATION &rhs_) const noexcept
    {
        return (x == static_cast<T>(rhs_));
    }

    constexpr inline operator ORIENTATION() const noexcept
    {
        return static_cast<ORIENTATION>(x);
    }

    constexpr inline ORIENTATION getOrientation() const noexcept
    {
        return static_cast<ORIENTATION>(ValueToOrientation(x));
    }

    template<Numeric TR>
    constexpr inline auto operator+(const Vector2<TR>& rhs) const noexcept -> Vector2<decltype(x+rhs.x)>
    {
        return { x + rhs.x, y + rhs.y };
    }

    template<Numeric TR>
    constexpr inline auto operator-(const Vector2<TR>& rhs) const noexcept -> Vector2<decltype(x-rhs.x)>
    {
        return { x - rhs.x, y - rhs.y };
    }

    constexpr inline Vector2<T> operator-() const noexcept
    {
        return { -x, -y };
    }

    template<Numeric TR>
    constexpr inline auto operator*(const TR& num) const noexcept -> Vector2<decltype(x*num)>
    {
        return { x * num, y * num };
    }

    template<Numeric TR>
    constexpr inline auto operator/(const TR& num) const noexcept -> Vector2<decltype(x/num)>
    {
        assert(num != 0);

        return { x / num, y / num };
    }

    template<Numeric TR>
    constexpr inline Vector2<T> &operator+=(const Vector2<TR>& rhs) noexcept
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    template<Numeric TR>
    constexpr inline Vector2<T> operator-=(const Vector2<TR>& rhs) noexcept
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    template<Numeric TR>
    constexpr inline Vector2<T> operator*=(const TR& num) noexcept
    {
        x *= num;
        y *= num;
        return *this;
    }

    template<Numeric TR>
    constexpr inline Vector2<T> operator/=(const TR& num) noexcept
    {
        x /= num;
        y /= num;
        return *this;
    }

    constexpr inline auto length() const noexcept
    {
        return sqrt(x * x + y * y);
    }

    constexpr inline auto sqLength() const noexcept
    {
        return x * x + y * y;
    }

    [[nodiscard]]
    constexpr inline auto abs() const noexcept
    {
        return Vector2{std::abs(x), std::abs(y)};
    }

    [[nodiscard]]
    constexpr inline auto pow(float pow_) const noexcept
    {
        return Vector2{std::pow(x, pow_), std::pow(y, pow_)};
    }

    constexpr inline auto square() const noexcept
    {
        return x * y;
    }

    [[nodiscard]]
    constexpr inline Vector2<decltype(x / sqrt(x))> normalised() const noexcept
    {
        float l = sqrt(x * x + y * y);
        if (l == 0.0f)
            return *this;

        return {x / l, y / l};
    }

    template<Numeric TR>
    [[nodiscard]]
    constexpr inline auto mulComponents(const Vector2<TR>& rhs) const noexcept -> Vector2<decltype(x * rhs.x)>
    {
        return { x * rhs.x, y * rhs.y };
    }

    template<Numeric TR>
    [[nodiscard]]
    constexpr inline auto mulComponents(const TR &x_, const TR &y_) const noexcept -> Vector2<decltype(x * x_)>
    {
        return { x * x_, y * y_ };
    }

    template<Numeric TR>
    constexpr inline auto dot(const Vector2<TR>& rhs) const noexcept -> decltype(x * rhs.x)
    {
        return x * rhs.x + y * rhs.y;
    }

    template<Numeric TR>
    constexpr inline auto cross(const Vector2<TR>& rhs) const noexcept -> decltype(x * rhs.x)
    {
        return x * rhs.y - y * rhs.x;
    }

    template<bool ON_ZEROES = true, Numeric T2>
    constexpr inline bool areAlignedOnX(const Vector2<T2> &rhs_) const noexcept
    {
        return utils::sameSign<ON_ZEROES, T, T2>(x, rhs_.x);
    }

    template<bool ON_ZEROES = true, Numeric T2>
    constexpr inline bool areAlignedOnY(const Vector2<T2> &rhs_) const noexcept
    {
        return utils::sameSign<ON_ZEROES, T, T2>(y, rhs_.y);
    }
};

template <Numeric T>
std::ostream& operator<< (std::ostream& out, const Vector2<T>& vec)
{
    out << vec.x << " " << vec.y;
    return out;
}

namespace utils
{
    template <Numeric T>
    inline Vector2<T> clamp(const Vector2<T>& val, const Vector2<T> &min, const Vector2<T> &max) noexcept
    {
        return {clamp(val.x, min.x, max.x), clamp(val.y, min.y, max.y)};
    }

    template <Numeric T>
    inline Vector2<T> limitVectorLength(const Vector2<T>& val, const T &limit)
    {
        auto curlen = val.length();
        if (curlen > limit)
        {
            return val / (curlen / limit);
        }

        return val;
    }

    template <Numeric T, Numeric aT>
    inline Vector2<T> lerp(const Vector2<T> &min, const Vector2<T> &max, const aT &alpha) noexcept
    {
        return {min + (max - min) * alpha};
    }

    template <Numeric T>
    inline std::string toString(const Vector2<T> &v_)
    {
        std::stringstream s;
        s << "{" << v_.x << ", " << v_.y << "}";
        return s.str();
    }

    inline float distToLineSegment(const Vector2<float> &lineP1_, const Vector2<float> &lineP2_, const Vector2<float> &point_) noexcept
    {
        auto dir1 = lineP2_ - lineP1_;
        auto p1 = point_ - lineP1_;

        if (dir1.dot(p1) <= 0.0f)
            return p1.length();

        auto dir2 = lineP1_ - lineP2_;
        auto p2 = point_ - lineP2_;

        if (dir2.dot(p2) <= 0.0f)
            return p2.length();

        Vector2<float> normal = Vector2{dir1.y, -dir1.x}.normalised();
        return abs(normal.dot(p1));
    }
}

// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
inline std::pair<float, float> overlaps(const std::pair<Vector2<float>, Vector2<float>> &con1_, const std::pair<Vector2<float>, Vector2<float>> &con2_) noexcept
{
    auto r = con1_.second - con1_.first;
    auto s = con2_.second - con2_.first;
    auto diff = con2_.first - con1_.first;
    auto divider = r.cross(s);

    if (divider == 0) // lines are parallel (and possibly collinear)
        return {-1.0f, -1.0f};

    auto u = (diff).cross(r) / divider;
    auto t = (diff).cross(s) / divider;

    return {t, u};
}

#endif

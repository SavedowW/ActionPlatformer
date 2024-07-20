#ifndef VECTOR2_H_
#define VECTOR2_H_
#include <cmath>
#include <iostream>
#include "Utils.h"
#include <type_traits>
#include <map>

enum class ORIENTATION {RIGHT = 0, LEFT = 1, UNSPECIFIED = 2};
inline const std::map<ORIENTATION, const char *> OrientationNames {
    {ORIENTATION::RIGHT, "RIGHT"},
    {ORIENTATION::LEFT, "LEFT"},
    {ORIENTATION::UNSPECIFIED, "UNSPECIFIED"},
};

template <typename T>
struct Vector2
{
    T x, y;
    constexpr inline Vector2(T nx = 0, T ny = 0)
    {
        static_assert(std::is_arithmetic_v<T>, "Type T must be an arithmetic type");

        x = nx;
        y = ny;
    }

    template <typename TR>
    constexpr inline Vector2(const Vector2<TR> &rhs)
    {
        x = rhs.x;
        y = rhs.y;
    }

    template <typename TR>
    constexpr inline Vector2<T> &operator=(const Vector2<TR> &rhs)
    {
        x = rhs.x;
        y = rhs.y;
        return *this;
    }

    template <typename TR>
    constexpr inline bool operator==(const Vector2<TR> &rhs) const
    {
        return (x == rhs.x && y == rhs.y);
    }

    constexpr inline bool operator==(const ORIENTATION &rhs_) const
    {
        return (x > 0 && rhs_ == ORIENTATION::RIGHT) || (x < 0 && rhs_ == ORIENTATION::LEFT) || (x == 0 && rhs_ == ORIENTATION::UNSPECIFIED);
    }

    template<ORIENTATION DEFAULT = ORIENTATION::UNSPECIFIED>
    constexpr inline bool getOrientation() const
    {
        if (x > 0)
            return ORIENTATION::RIGHT;
        else if (x < 0)
            return ORIENTATION::LEFT;
        else
            return DEFAULT;
    }

    template<typename TR>
    constexpr inline auto operator+(const Vector2<TR>& rhs) const -> Vector2<decltype(x+rhs.x)>
    {
        return { x + rhs.x, y + rhs.y };
    }

    template<typename TR>
    constexpr inline auto operator-(const Vector2<TR>& rhs) const -> Vector2<decltype(x-rhs.x)>
    {
        return { x - rhs.x, y - rhs.y };
    }

    constexpr inline Vector2<T> operator-() const
    {
        return { -x, -y };
    }

    template<typename TR>
    constexpr inline auto operator*(const TR& num) const -> Vector2<decltype(x*num)>
    {
        return { x * num, y * num };
    }

    template<typename TR>
    constexpr inline auto operator/(const TR& num) const -> Vector2<decltype(x/num)>
    {
        return { x / num, y / num };
    }

    template<typename TR>
    constexpr inline Vector2<T> &operator+=(const Vector2<TR>& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    template<typename TR>
    constexpr inline Vector2<T> operator-=(const Vector2<TR>& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    template<typename TR>
    constexpr inline Vector2<T> operator*=(const TR& num)
    {
        x *= num;
        y *= num;
        return *this;
    }

    template<typename TR>
    constexpr inline Vector2<T> operator/=(const TR& num)
    {
        x /= num;
        y /= num;
        return *this;
    }

    constexpr inline auto getLen() const
    {
        return sqrt(x * x + y * y);
    }

    constexpr inline auto getSqLen() const
    {
        return x * x + y * y;
    }

    constexpr inline auto abs() const
    {
        return Vector2{std::abs(x), std::abs(y)};
    }

    constexpr inline auto pow(float pow_) const
    {
        return Vector2{std::pow(x, pow_), std::pow(y, pow_)};
    }

    constexpr inline auto square() const
    {
        return x * y;
    }


    constexpr inline Vector2<decltype(x / sqrt(x))> normalised() const
    {
        float l = sqrt(x * x + y * y);
        if (l == 0.0f)
            return *this;

        return {x / l, y / l};
    }

    template<typename TR>
    constexpr inline auto mulComponents(const Vector2<TR>& rhs) const -> Vector2<decltype(x * rhs.x)>
    {
        return { x * rhs.x, y * rhs.y };
    }

    template<bool ON_ZEROES = true, typename T2>
    constexpr inline bool areAlignedOnX(const Vector2<T2> &rhs_) const
    {
        return utils::sameSign<ON_ZEROES, T, T2>(x, rhs_.x);
    }

    template<bool ON_ZEROES = true, typename T2>
    constexpr inline bool areAlignedOnY(const Vector2<T2> &rhs_) const
    {
        return utils::sameSign<ON_ZEROES, T, T2>(y, rhs_.y);
    }
};

template <typename T>
std::ostream& operator<< (std::ostream& out, const Vector2<T>& vec)
{
    out << vec.x << " " << vec.y;
    return out;
}

//Only rectangle hitbox
struct Collider
{
    Vector2<float> m_center;
    Vector2<float> m_halfSize;
    
    template<typename T1, typename T2>
    constexpr inline Collider(const Vector2<T1> &center_, const Vector2<T2> &halfSize_) :
        m_center(center_),
        m_halfSize(halfSize_)
    {
    }

    public:
    constexpr inline Collider operator+(const Vector2<float>& rhs_) const
    {
        return { m_center + rhs_, m_halfSize };
    }

    constexpr inline float getLeftEdge() const
    {
        return m_center.x - m_halfSize.x;
    }

    constexpr inline float getRightEdge() const
    {
        return m_center.x + m_halfSize.x;
    }

    constexpr inline float getTopEdge() const
    {
        return m_center.y - m_halfSize.y;
    }

    constexpr inline float getBottomEdge() const
    {
        return m_center.y + m_halfSize.y;
    }

    //TODO: flags in output
    template<bool H_OVERLAP_ONLY, bool V_OVERLAP_ONLY>
    constexpr inline bool checkCollisionWith(const Collider& rhs_) const
    {
        auto delta = (m_center - rhs_.m_center).abs();
        auto maxdst = m_halfSize + rhs_.m_halfSize;
        bool hor = (H_OVERLAP_ONLY ? delta.x < maxdst.x : delta.x <= maxdst.x);
        bool ver = (V_OVERLAP_ONLY ? delta.y < maxdst.y : delta.y <= maxdst.y);

        return hor & ver;
    }

    
    /* TODO: template params and flags in output
    constexpr inline int isWithinHorizontalBounds(float leftBound_, float rightBound_) const
    {
        if (getLeftEdge() < leftBound_)
            return -1;
        
        if (getRightEdge() > rightBound_)
            return 1;

        return 0;
    }
    */
    
    constexpr inline float rangeToLeftBound(float leftBound_) const
    {
        return getLeftEdge() - leftBound_;
    }

    constexpr inline float rangeToRightBound(float rightBound_) const
    {
        return rightBound_ - getRightEdge();
    }

    constexpr inline Vector2<float> getTopLeft() const
    {
        return m_center - m_halfSize;
    }

    constexpr inline Vector2<float> getSize() const
    {
        return m_halfSize * 2;
    }

    constexpr inline float getSquare() const
    {
        return (m_halfSize * 2).square();
    }

    constexpr inline float getOwnOverlapPortion(const Collider &rhs_) const
    {
        auto size = m_halfSize + rhs_.m_halfSize - (rhs_.m_center - m_center).abs();
        if (size.x <= 0 || size.y <= 0)
            return 0;

        return (size.x * size.y) / getSquare();
    }
    
};

inline std::ostream& operator<< (std::ostream& out_, const Collider& cld_)
{
    out_ << "{ " << cld_.getLeftEdge() << ", " << cld_.getTopEdge() << ", " << cld_.getRightEdge() << ", " << cld_.getBottomEdge() << " }";
    return out_;
}

namespace utils
{
    template <typename T>
    inline Vector2<T> clamp(const Vector2<T>& val, const Vector2<T> &min, const Vector2<T> &max)
    {
        return {clamp(val.x, min.x, max.x), clamp(val.y, min.y, max.y)};
    }

    template <typename T>
    inline Vector2<T> limitVectorLength(const Vector2<T>& val, const T &limit)
    {
        auto curlen = val.getLen();
        if (curlen > limit)
        {
            return val / (curlen / limit);
        }

        return val;
    }

    template <typename T, typename aT>
    inline Vector2<T> lerp(const Vector2<T> &min, const Vector2<T> &max, const aT &alpha)
    {
        return {min + (max - min) * alpha};
    }

    inline std::vector<Vector2<int>> getAreaEdgePoints(int w_, int h_, const Vector2<int> &origin_, int len_)
    {
        int minx = origin_.x - len_;
        int maxx = origin_.x + len_;

        int miny = origin_.y - len_;
        int maxy = origin_.y + len_;

        std::vector<Vector2<int>> points;

        for (int x = minx; x <= maxx; ++x)
        {
            for (int y : {miny, maxy})
            {
                if (x >= 0 && x < w_ && y >= 0 && y < h_)
                    points.push_back({x, y});
            }
        }

        for (int y = miny + 1; y < maxy; ++y)
        {
            for (int x : {minx, maxx})
            {
                if (x >= 0 && x < w_ && y >= 0 && y < h_)
                    points.push_back({x, y});
            }
        }

        return points;
    }

    template <typename T>
    inline std::string toString(const Vector2<T> &v_)
    {
        std::stringstream s;
        s << "{" << v_.x << ", " << v_.y << "}";
        return s.str();
    }
}

#endif
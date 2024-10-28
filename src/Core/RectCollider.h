#ifndef RECT_COLLIDER_H_
#define RECT_COLLIDER_H_
#include "Vector2.h"

enum class OverlapResult : uint8_t
{
    NONE = 0,
    OVERLAP_X = 0b01,
    OVERLAP_Y = 0b10,
    OVERLAP_BOTH = 0b11
};

constexpr inline OverlapResult operator|(const OverlapResult& lhs_, const OverlapResult& rhs_)
{
    return static_cast<OverlapResult>(static_cast<uint8_t>(lhs_) | static_cast<uint8_t>(rhs_));
}

constexpr inline OverlapResult operator&(const OverlapResult& lhs_, const OverlapResult& rhs_)
{
    return static_cast<OverlapResult>(static_cast<uint8_t>(lhs_) & static_cast<uint8_t>(rhs_));
}

constexpr inline OverlapResult &operator|=(OverlapResult& lhs_, const OverlapResult& rhs_)
{
    return lhs_ = lhs_ | rhs_;
}

constexpr bool checkCollision(const OverlapResult &res_, const OverlapResult &expected_)
{
    return (res_ & expected_) == expected_;
}

//Only rectangle hitbox
struct Collider
{
    Vector2<float> m_center;
    Vector2<float> m_halfSize;
    
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

    //First 6 bits describe horizontal overlap, second 6 bits - vertical
    inline OverlapResult checkOverlap(const Collider& rhs_) const
    {
        auto delta = (rhs_.m_center - m_center).abs();
        OverlapResult res = OverlapResult::NONE;

        if (delta.x < rhs_.m_halfSize.x + m_halfSize.x)
            res |= OverlapResult::OVERLAP_X;

        if (delta.x < rhs_.m_halfSize.x + m_halfSize.x)
            res |= OverlapResult::OVERLAP_Y;

        return res;
    }
    
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

    constexpr inline Collider getOverlapArea(const Collider &rhs_) const
    {
        Vector2<float> tl_max{std::max(getLeftEdge(), rhs_.getLeftEdge()), std::max(getTopEdge(), rhs_.getTopEdge())};
        Vector2<float> br_min{std::min(getRightEdge(), rhs_.getRightEdge()), std::min(getBottomEdge(), rhs_.getBottomEdge())};

        return Collider((tl_max + br_min) / 2, (br_min - tl_max) / 2);
    }

    constexpr inline bool includesPoint(const Vector2<float> point_) const
    {
        auto delta = (point_ - m_center).abs();
        return delta.x <= m_halfSize.x && delta.y <= m_halfSize.y;
    }

};

inline std::ostream& operator<< (std::ostream& out_, const Collider& cld_)
{
    out_ << "{ " << cld_.getLeftEdge() << ", " << cld_.getTopEdge() << ", " << cld_.getRightEdge() << ", " << cld_.getBottomEdge() << " }";
    return out_;
}

#endif
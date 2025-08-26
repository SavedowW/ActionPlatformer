#ifndef RECT_COLLIDER_H_
#define RECT_COLLIDER_H_
#include "StateCommon.h"
#include "Vector2.hpp"

enum class OverlapResult : uint8_t
{
    NONE = 0,
    OVERLAP_X = 0b01,
    OVERLAP_Y = 0b10,
    OVERLAP_BOTH = 0b11
};

//Only rectangle hitbox
struct Collider
{
    Vector2<int> m_topLeft;
    Vector2<int> m_size;
    
    public:
    constexpr Collider operator+(const Vector2<int>& rhs_) const
    {
        return { .m_topLeft=m_topLeft + rhs_ + Vector2{1, 1}, .m_size=m_size };
    }

    constexpr int getLeftEdge() const
    {
        return m_topLeft.x;
    }

    constexpr int getRightEdge() const
    {
        return m_topLeft.x + m_size.x - 1;
    }

    constexpr int getTopEdge() const
    {
        return m_topLeft.y;
    }

    constexpr int getBottomEdge() const
    {
        return m_topLeft.y + m_size.y - 1;
    }

    Flag<OverlapResult> checkOverlap(const Collider& rhs_) const
    {
        Flag<OverlapResult> res = OverlapResult::NONE;

        if ( (getRightEdge() >= rhs_.getLeftEdge()) && (getLeftEdge() <= rhs_.getRightEdge()) )
            res |= OverlapResult::OVERLAP_X;

        if ( (getBottomEdge() >= rhs_.getTopEdge()) && (getTopEdge() <= rhs_.getBottomEdge()) )
            res |= OverlapResult::OVERLAP_Y;

        return res;
    }

    constexpr int getSquare() const
    {
        return m_size.x * m_size.y;
    }

    constexpr float getOwnOverlapPortion(const Collider &rhs_) const
    {
        Vector2<int> tl_max{std::max(getLeftEdge(), rhs_.getLeftEdge()), std::max(getTopEdge(), rhs_.getTopEdge())};
        Vector2<int> br_min{std::min(getRightEdge(), rhs_.getRightEdge()), std::min(getBottomEdge(), rhs_.getBottomEdge())};

        if (br_min.x <= tl_max.x || br_min.y <= tl_max.y)
            return 0;

        return float((br_min - tl_max + Vector2{1, 1}).square()) / m_size.square();
    }

    constexpr Collider getOverlapArea(const Collider &rhs_) const
    {
        Vector2<int> tl_max{std::max(getLeftEdge(), rhs_.getLeftEdge()), std::max(getTopEdge(), rhs_.getTopEdge())};
        Vector2<int> br_min{std::min(getRightEdge(), rhs_.getRightEdge()), std::min(getBottomEdge(), rhs_.getBottomEdge())};

        return Collider(tl_max, br_min - tl_max + Vector2{1, 1});
    }

    constexpr bool includesPoint(const Vector2<int> point_) const
    {
        auto delta = (point_ - m_topLeft);
        return delta.x < m_size.x && delta.y < m_size.y && delta.x >= 0 && delta.y >= 0;
    }

};

inline std::ostream& operator<< (std::ostream& out_, const Collider& cld_)
{
    out_ << "{ " << cld_.getLeftEdge() << ", " << cld_.getTopEdge() << ", " << cld_.getRightEdge() << ", " << cld_.getBottomEdge() << " }";
    return out_;
}

#endif

#ifndef COLLIDER_H_
#define COLLIDER_H_
#include "StateCommon.h"
#include "Vector2.hpp"
#include "RectCollider.h"

/*
        |\
        | \
        |  \
        |   \
        |    \
        |     \
        |      |
        |      |
        |      |
        |      |
        |      |
        --------
*/

class SlopeCollider
{
public:
    SlopeCollider() = default;
    SlopeCollider(const Vector2<int> &topLeft_, const Vector2<int> &topRight_, int bottomHeight_);
    void set(const Vector2<int> &topLeft_, const Vector2<int> &topRight_, int bottomHeight_);

    int leftX() const noexcept { return m_topLeft.x; }
    int rightX() const noexcept { return m_topRight.x; }
    int leftY() const noexcept { return m_topLeft.y; }
    int rightY() const noexcept { return m_topRight.y; }
    int bottomY() const noexcept { return m_bottomHeight; }
    const Vector2<int> &topLeft() const noexcept { return m_topLeft; };
    int highestPoint() const noexcept { return m_highestSlopePoint; };

    bool isFlat() const noexcept { return m_topLeft.y == m_topRight.y; };
    float topAngleCoef() const noexcept { return m_topAngleCoef; };

    Flag<OverlapResult> checkOverlap(const Collider &cld_) const;
    Flag<OverlapResult> checkOverlap(const Collider &cld_, int &highestPoint_) const;

    int getHeightAt(int x) const;

    // Rightmost X coordinate with Y at the bottom line or above
    int getMostRightAt(const Collider &cld_) const;
    int getMostLeftAt(const Collider &cld_) const;

    [[nodiscard]]
    SlopeCollider movedBy(const Vector2<int> &offset_) const noexcept;

private:
    Vector2<int> m_topLeft;
    Vector2<int> m_topRight;
    int m_bottomHeight = 0;
    int m_lowestSlopePoint = 0;
    int m_highestSlopePoint = 0;
    float m_topAngleCoef = 0.0f;
};


#endif

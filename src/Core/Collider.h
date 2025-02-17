#ifndef COLLIDER_H_
#define COLLIDER_H_
#include "Vector2.h"
#include "RectCollider.h"

/*
        p0
        |\
        | \
        |  \
        |   \
        |    \
        |     \
size.y  |      | p1
        |      |
        |      | size.y - size.x * topAngle
        |      |
        |      |
     p3 -------- p2
         size.x
*/

struct SlopeCollider
{
    SlopeCollider() = default;
    SlopeCollider(const Vector2<int> &tlPos_, const Vector2<int> &size_, float topAngleCoef_);
    SlopeCollider(const Vector2<int> (&vertices_)[4]);
    void set(const Vector2<int> &tlPos_, const Vector2<int> &size_, float topAngleCoef_);
    void set(const Vector2<int> (&vertices_)[4]);
    Vector2<int> m_tlPos;
    Vector2<int> m_size;
    float m_topAngleCoef = 0.0f;
    int m_highestSlopePoint = 0.0f;
    int m_lowestSlopePoint = 0.0f;
    bool m_hasSlope = false;
    bool m_hasBox = false;

    Vector2<int> m_points[4];
    void generatePoints();

    OverlapResult checkOverlap(const Collider &cld_) const;
    OverlapResult checkOverlap(const Collider &cld_, int &highestPoint_) const;

    int getOrientationDir() const;

    int getHeightAt(int x) const;

    int getMostRightAt(const Collider &cld_) const;
    int getMostLeftAt(const Collider &cld_) const;

    bool containsPoint(const Vector2<int> &point_) const;
};


#endif

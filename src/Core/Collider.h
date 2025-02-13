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
    SlopeCollider(const Vector2<float> &tlPos_, const Vector2<float> &size_, float topAngleCoef_);
    SlopeCollider(const Vector2<float> (&vertices_)[4]);
    void set(const Vector2<float> &tlPos_, const Vector2<float> &size_, float topAngleCoef_);
    void set(const Vector2<float> (&vertices_)[4]);
    Vector2<float> m_tlPos;
    Vector2<float> m_size;
    float m_topAngleCoef = 0.0f;
    float m_highestSlopePoint = 0.0f;
    float m_lowestSlopePoint = 0.0f;
    bool m_hasSlope = false;
    bool m_hasBox = false;

    Vector2<float> m_points[4];
    void generatePoints();

    OverlapResult checkOverlap(const Collider &cld_) const;
    OverlapResult checkOverlap(const Collider &cld_, float &highestPoint_) const;

    int getOrientationDir() const;

    float getHeightAt(float x) const;

    float getMostRightAt(const Collider &cld_) const;
    float getMostLeftAt(const Collider &cld_) const;

    bool containsPoint(const Vector2<float> &point_) const;
};


#endif

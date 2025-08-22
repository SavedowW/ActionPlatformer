#ifndef COLLIDER_H_
#define COLLIDER_H_
#include "StateCommon.h"
#include "Vector2.hpp"
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

template<typename VT>
struct Quad
{
    // Top Left
    Vector2<int> tl;

    // Top Right
    Vector2<int> tr;

    // Bottom Right
    Vector2<int> br;

    // Bottom Left
    Vector2<int> bl;
};

struct SlopeCollider
{
    SlopeCollider() = default;
    SlopeCollider(const Vector2<int> &tlPos_, const Vector2<int> &size_, float topAngleCoef_);
    SlopeCollider(const Quad<int> &vertices_);
    void set(const Vector2<int> &tlPos_, const Vector2<int> &size_, float topAngleCoef_);
    void set(const Quad<int> &vertices_);
    Vector2<int> m_tlPos;
    Vector2<int> m_size;
    float m_topAngleCoef = 0.0f;
    int m_highestSlopePoint = 0;
    int m_lowestSlopePoint = 0;
    bool m_hasSlope = false;
    bool m_hasBox = false;

    Quad<int> m_points;
    void generatePoints();

    Flag<OverlapResult> checkOverlap(const Collider &cld_) const;
    Flag<OverlapResult> checkOverlap(const Collider &cld_, int &highestPoint_) const;

    int getOrientationDir() const;

    int getHeightAt(int x) const;

    int getMostRightAt(const Collider &cld_) const;
    int getMostLeftAt(const Collider &cld_) const;

    bool containsPoint(const Vector2<int> &point_) const;
};


#endif

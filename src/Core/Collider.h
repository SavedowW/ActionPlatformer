#ifndef COLLIDER_H_
#define COLLIDER_H_
#include "Vector2.h"

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

    inline utils::OverlapResult getFullCollisionWith(const Collider &cld_, float &highestPoint_) const
    {
        float leftEdge = cld_.getLeftEdge();
        float rightEdge = cld_.getRightEdge();
        float topEdge = cld_.getTopEdge();
        float bottomEdge = cld_.getBottomEdge();

        auto hRes = utils::getOverlap<0>(m_points[0].x, m_points[1].x, leftEdge, rightEdge);

        float highest = 0;

        if (hRes == utils::OverlapResult::NONE)
            return hRes;

        if (m_topAngleCoef == 0) // Flat
        {
            highest = m_highestSlopePoint;
        }
        else if (m_topAngleCoef > 0) // Goes down to the right
        {
            auto x = std::max(m_tlPos.x, leftEdge);
            highest = getHeightAt(x);
        }
        else // Goes up to the right
        {
            auto x = std::min(m_points[1].x, rightEdge);
            highest = getHeightAt(x);
        }

        hRes |= utils::getOverlap<6>(highest, m_points[2].y, topEdge, bottomEdge);

        if ((hRes & utils::OverlapResult::OVERLAP_X) && (hRes & utils::OverlapResult::OVERLAP_Y))
        {
            hRes |= utils::OverlapResult::BOTH_OVERLAP;
        }
        else if ((hRes & utils::OverlapResult::TOUCH_X) && (hRes & utils::OverlapResult::TOUCH_Y))
        {
            hRes |= utils::OverlapResult::BOTH_TOUCH;
        }
        else if ((hRes & utils::OverlapResult::OOT_X) && (hRes & utils::OverlapResult::OOT_Y))
        {
            hRes |= utils::OverlapResult::BOTH_OOT;
        }

        if (!!(hRes & utils::OverlapResult::OOT_Y))
        {
            highestPoint_ = highest;

            if (bottomEdge >= m_lowestSlopePoint)
                hRes |= utils::OverlapResult::OOT_BOX;

            if (topEdge <= m_lowestSlopePoint)
                hRes |= utils::OverlapResult::OOT_SLOPE;
        }

        return hRes;
    }

    int getOrientationDir() const;

    float getTopHeight(const Collider &cld_, utils::OverlapResult horOverlapType_) const;

    float getHeightAt(float x) const;

    float getMostRightAt(const Collider &cld_);
    float getMostLeftAt(const Collider &cld_);
};


#endif
#include "Collider.h"

SlopeCollider::SlopeCollider(const Vector2<float> &tlPos_, const Vector2<float> &size_, float topAngleCoef_) :
    m_tlPos(tlPos_),
    m_size(size_),
    m_topAngleCoef(std::min(topAngleCoef_, 1.0f))
{
    generatePoints();
}

SlopeCollider::SlopeCollider(const Vector2<float> (&vertices_)[4])
{
    set(vertices_);
}

void SlopeCollider::set(const Vector2<float> &tlPos_, const Vector2<float> &size_, float topAngleCoef_)
{
    m_tlPos = tlPos_;
    m_size = size_;
    m_topAngleCoef = std::min(topAngleCoef_, 1.0f);

    generatePoints();
}

void SlopeCollider::set(const Vector2<float> (&vertices_)[4])
{
    for (int i = 0; i < 4; ++i)
        m_points[i] = vertices_[i];

    m_tlPos = m_points[0];
    m_size = m_points[2] - m_points[0];
    m_topAngleCoef = (m_points[1].y - m_points[0].y) / m_size.x;

    m_highestSlopePoint = std::min(m_points[0].y, m_points[1].y);
    m_lowestSlopePoint = std::max(m_points[0].y, m_points[1].y);
    m_hasSlope = m_points[1].y - m_points[0].y;
    m_hasBox = m_lowestSlopePoint - m_points[2].y;
}

void SlopeCollider::generatePoints()
{
    m_points[0] = m_tlPos;
    m_points[1] = m_tlPos + Vector2{m_size.x, m_size.x * m_topAngleCoef};
    m_points[2] = m_tlPos + m_size;
    m_points[3] = m_tlPos + Vector2{0.0f, m_size.y};

    m_highestSlopePoint = std::min(m_points[0].y, m_points[1].y);
    m_lowestSlopePoint = std::max(m_points[0].y, m_points[1].y);
    m_hasSlope = m_points[1].y - m_points[0].y;
    m_hasBox = m_lowestSlopePoint - m_points[2].y;
}

CollisionResult SlopeCollider::checkOverlap(const Collider &cld_) const
{
    float leftEdge = cld_.getLeftEdge();
    float rightEdge = cld_.getRightEdge();
    float topEdge = cld_.getTopEdge();
    float bottomEdge = cld_.getBottomEdge();

    if (m_topAngleCoef > 0)
    {
        float highestx = 0.0f;
        CollisionResult res = CollisionResult::NONE;

        if (leftEdge > m_points[0].x && rightEdge < m_points[1].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = leftEdge;
        }
        else if (leftEdge <= m_points[0].x && rightEdge > m_points[0].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = m_points[0].x;
        }
        else if (leftEdge < m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = leftEdge;
        }
        else
            return CollisionResult::NONE;
        
        auto highest = getHeightAt(highestx);
        if (bottomEdge <= highest || topEdge >= m_points[2].y)
            return res;
        
        return CollisionResult::OVERLAP_BOTH;
    }
    else if (m_topAngleCoef < 0)
    {
        float highestx = 0.0f;
        CollisionResult res = CollisionResult::NONE;

        if (leftEdge > m_points[0].x && rightEdge < m_points[1].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = rightEdge;
        }
        else if (leftEdge < m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = m_points[1].x;
        }
        else if (leftEdge <= m_points[0].x && rightEdge > m_points[0].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = rightEdge;
        }
        else
            return CollisionResult::NONE;
        
        auto highest = getHeightAt(highestx);
        if (bottomEdge <= highest || topEdge >= m_points[2].y)
            return res;
        
        return CollisionResult::OVERLAP_BOTH;
    }
    else
    {
        auto halfSize = m_size / 2.0f;
        auto center = m_tlPos + halfSize;

        auto delta = (cld_.m_center - center).abs();
        CollisionResult res = CollisionResult::NONE;

        if (delta.x < cld_.m_halfSize.x + halfSize.x)
            res |= CollisionResult::OVERLAP_X;

        if (delta.y < cld_.m_halfSize.y + halfSize.y)
        {
            res |= CollisionResult::OVERLAP_Y;
        }

        return res;
    }
}

CollisionResult SlopeCollider::checkOverlap(const Collider &cld_, float &highestPoint_) const
{
    float leftEdge = cld_.getLeftEdge();
    float rightEdge = cld_.getRightEdge();
    float topEdge = cld_.getTopEdge();
    float bottomEdge = cld_.getBottomEdge();

    if (m_topAngleCoef > 0)
    {
        float highestx = 0.0f;
        CollisionResult res = CollisionResult::NONE;

        if (leftEdge > m_points[0].x && rightEdge < m_points[1].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = leftEdge;
        }
        else if (leftEdge <= m_points[0].x && rightEdge > m_points[0].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = m_points[0].x;
        }
        else if (leftEdge < m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = leftEdge;
        }
        else
            return CollisionResult::NONE;
        
        highestPoint_ = getHeightAt(highestx);
        if (bottomEdge <= highestPoint_ || topEdge >= m_points[2].y)
            return res;
        
        return CollisionResult::OVERLAP_BOTH;
    }
    else if (m_topAngleCoef < 0)
    {
        float highestx = 0.0f;
        CollisionResult res = CollisionResult::NONE;

        if (leftEdge > m_points[0].x && rightEdge < m_points[1].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = rightEdge;
        }
        else if (leftEdge < m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = m_points[1].x;
        }
        else if (leftEdge <= m_points[0].x && rightEdge > m_points[0].x)
        {
            res |= CollisionResult::OVERLAP_X;
            highestx = rightEdge;
        }
        else
            return CollisionResult::NONE;
        
        highestPoint_ = getHeightAt(highestx);
        if (bottomEdge <= highestPoint_ || topEdge >= m_points[2].y)
            return res;
        
        return CollisionResult::OVERLAP_BOTH;
    }
    else
    {
        auto halfSize = m_size / 2.0f;
        auto center = m_tlPos + halfSize;

        auto delta = (cld_.m_center - center).abs();
        CollisionResult res = CollisionResult::NONE;

        if (delta.x < cld_.m_halfSize.x + halfSize.x)
        {
            highestPoint_ = m_points[0].y;
            res |= CollisionResult::OVERLAP_X;
        }

        if (delta.y < cld_.m_halfSize.y + halfSize.y)
            res |= CollisionResult::OVERLAP_Y;

        return res;
    }
}

int SlopeCollider::getOrientationDir() const
{
    return ValueToOrientationInt(-m_topAngleCoef);
}

float SlopeCollider::getTopHeight(const Collider &cld_, utils::OverlapResult horOverlapType_) const
{
    if (m_topAngleCoef == 0) // Flat
    {
        return m_highestSlopePoint;
    }
    else if (m_topAngleCoef > 0) // Goes down to the right
    {
        auto x = std::max(m_tlPos.x, cld_.getLeftEdge());
        return getHeightAt(x);
    }
    else // Goes up to the right
    {
        auto x = std::min(m_points[1].x, cld_.getRightEdge());
        return getHeightAt(x);
    }
}

float SlopeCollider::getHeightAt(float x) const
{
    return m_tlPos.y + m_topAngleCoef * (x - m_tlPos.x);
}

float SlopeCollider::getMostRightAt(const Collider &cld_)
{
    if (m_topAngleCoef <= 0) // Flat or up to the right
    {
        return m_points[1].x;
    }
    else // Goes down to the right
    {
        auto bot = cld_.getBottomEdge();
        if (bot < m_lowestSlopePoint)
        {
            return m_tlPos.x + (bot - m_tlPos.y) * m_topAngleCoef;
        }
        else
            return m_points[1].x;
    }
}

float SlopeCollider::getMostLeftAt(const Collider &cld_)
{
    if (m_topAngleCoef >= 0) // Flat or down to the right
    {
        return m_points[0].x;
    }
    else // Goes down to the right
    {
        auto bot = cld_.getBottomEdge();
        if (bot < m_lowestSlopePoint)
        {
            return m_tlPos.x + (bot - m_tlPos.y) * m_topAngleCoef;
        }
        else
            return m_points[0].x;
    }
}

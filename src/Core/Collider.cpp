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

OverlapResult SlopeCollider::checkOverlap(const Collider &cld_) const
{
    float leftEdge = cld_.getLeftEdge();
    float rightEdge = cld_.getRightEdge();
    float topEdge = cld_.getTopEdge();
    float bottomEdge = cld_.getBottomEdge();

    if (m_topAngleCoef > 0)
    {
        float highesty = 0.0f;
        OverlapResult res = OverlapResult::NONE;

        if (leftEdge > m_points[0].x && rightEdge < m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(leftEdge);
        }
        else if (leftEdge <= m_points[0].x && rightEdge > m_points[0].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = m_points[0].y;
        }
        else if (leftEdge < m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(leftEdge);
        }
        else
            return OverlapResult::NONE;
        
        if (bottomEdge <= highesty || topEdge >= m_points[2].y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    else if (m_topAngleCoef < 0)
    {
        float highesty = 0.0f;
        OverlapResult res = OverlapResult::NONE;

        if (leftEdge > m_points[0].x && rightEdge < m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(rightEdge);
        }
        else if (leftEdge < m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = m_points[1].y;
        }
        else if (leftEdge <= m_points[0].x && rightEdge > m_points[0].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(rightEdge);
        }
        else
            return OverlapResult::NONE;
        
        if (bottomEdge <= highesty || topEdge >= m_points[2].y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    else
    {
        auto halfSize = m_size / 2.0f;
        auto center = m_tlPos + halfSize;

        auto delta = (cld_.m_center - center).abs();
        OverlapResult res = OverlapResult::NONE;

        if (delta.x < cld_.m_halfSize.x + halfSize.x)
            res |= OverlapResult::OVERLAP_X;

        if (delta.y < cld_.m_halfSize.y + halfSize.y)
        {
            res |= OverlapResult::OVERLAP_Y;
        }

        return res;
    }
}

OverlapResult SlopeCollider::checkOverlap(const Collider &cld_, float &highestPoint_) const
{
    float leftEdge = cld_.getLeftEdge();
    float rightEdge = cld_.getRightEdge();
    float topEdge = cld_.getTopEdge();
    float bottomEdge = cld_.getBottomEdge();

    if (m_topAngleCoef > 0)
    {
        float highesty = 0.0f;
        OverlapResult res = OverlapResult::NONE;

        if (leftEdge > m_points[0].x && rightEdge < m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(leftEdge);
        }
        else if (leftEdge <= m_points[0].x && rightEdge > m_points[0].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = m_points[0].y;
        }
        else if (leftEdge < m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(leftEdge);
        }
        else
            return OverlapResult::NONE;
        
        highestPoint_ = highesty;
        if (bottomEdge <= highesty || topEdge >= m_points[2].y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    else if (m_topAngleCoef < 0)
    {
        float highesty = 0.0f;
        OverlapResult res = OverlapResult::NONE;

        if (leftEdge > m_points[0].x && rightEdge < m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(rightEdge);
        }
        else if (leftEdge < m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = m_points[1].y;
        }
        else if (leftEdge <= m_points[0].x && rightEdge > m_points[0].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(rightEdge);
        }
        else
            return OverlapResult::NONE;
        
        highestPoint_ = highesty;
        if (bottomEdge <= highesty || topEdge >= m_points[2].y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    else
    {
        auto halfSize = m_size / 2.0f;
        auto center = m_tlPos + halfSize;

        auto delta = (cld_.m_center - center).abs();
        OverlapResult res = OverlapResult::NONE;

        if (delta.x < cld_.m_halfSize.x + halfSize.x)
        {
            highestPoint_ = m_points[0].y;
            res |= OverlapResult::OVERLAP_X;
        }

        if (delta.y < cld_.m_halfSize.y + halfSize.y)
            res |= OverlapResult::OVERLAP_Y;

        return res;
    }
}

int SlopeCollider::getOrientationDir() const
{
    return ValueToOrientationInt(-m_topAngleCoef);
}

float SlopeCollider::getHeightAt(float x) const
{
    return m_tlPos.y + m_topAngleCoef * (x - m_tlPos.x);
}

float SlopeCollider::getMostRightAt(const Collider &cld_) const
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

float SlopeCollider::getMostLeftAt(const Collider &cld_) const
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

bool SlopeCollider::containsPoint(const Vector2<float> &point_) const
{
    if (point_.x < m_points[0].x || point_.x > m_points[0].x)
        return false;

    auto highest = getHeightAt(point_.x);

    return point_.y >= highest && point_.y <= m_points[2].y;
}

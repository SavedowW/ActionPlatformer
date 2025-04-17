#include "Collider.h"

SlopeCollider::SlopeCollider(const Vector2<int> &tlPos_, const Vector2<int> &size_, float topAngleCoef_) :
    m_tlPos(tlPos_),
    m_size(size_),
    m_topAngleCoef(std::min(topAngleCoef_, 1.0f))
{
    generatePoints();
}

SlopeCollider::SlopeCollider(const Vector2<int> (&vertices_)[4])
{
    set(vertices_);
}

void SlopeCollider::set(const Vector2<int> &tlPos_, const Vector2<int> &size_, float topAngleCoef_)
{
    m_tlPos = tlPos_;
    m_size = size_;
    m_topAngleCoef = std::min(topAngleCoef_, 1.0f);

    generatePoints();
}

void SlopeCollider::set(const Vector2<int> (&vertices_)[4])
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
    m_points[1] = m_tlPos + Vector2<int>{m_size.x - 1, int((m_size.x - 1) * m_topAngleCoef)};
    m_points[2] = m_tlPos + m_size - Vector2{1, 1};
    m_points[3] = m_tlPos + Vector2{0, m_size.y - 1};

    m_highestSlopePoint = std::min(m_points[0].y, m_points[1].y);
    m_lowestSlopePoint = std::max(m_points[0].y, m_points[1].y);
    m_hasSlope = m_points[1].y - m_points[0].y;
    m_hasBox = m_lowestSlopePoint - m_points[2].y;
}

OverlapResult SlopeCollider::checkOverlap(const Collider &cld_) const
{
    int leftEdge = cld_.getLeftEdge();
    int rightEdge = cld_.getRightEdge();
    int topEdge = cld_.getTopEdge();
    int bottomEdge = cld_.getBottomEdge();

    if (m_topAngleCoef > 0)
    {
        int highesty = 0;
        OverlapResult res = OverlapResult::NONE;

        if (leftEdge >= m_points[0].x && rightEdge <= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(leftEdge);
        }
        else if (leftEdge <= m_points[0].x && rightEdge >= m_points[0].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = m_points[0].y;
        }
        else if (leftEdge <= m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(leftEdge);
        }
        else
            return OverlapResult::NONE;
        
        if (bottomEdge < highesty || topEdge > m_points[2].y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    else if (m_topAngleCoef < 0)
    {
        int highesty = 0.0f;
        OverlapResult res = OverlapResult::NONE;

        if (leftEdge >= m_points[0].x && rightEdge <= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(rightEdge);
        }
        else if (leftEdge <= m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = m_points[1].y;
        }
        else if (leftEdge <= m_points[0].x && rightEdge >= m_points[0].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(rightEdge);
        }
        else
            return OverlapResult::NONE;
        
        if (bottomEdge < highesty || topEdge > m_points[2].y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    else
    {
        OverlapResult res = OverlapResult::NONE;

        if (!( (rightEdge < m_points[0].x) || (leftEdge > m_points[1].x) ))
            res |= OverlapResult::OVERLAP_X;

        if (!( (bottomEdge < m_points[0].y) || (topEdge > m_points[3].y) ))
            res |= OverlapResult::OVERLAP_Y;

        return res;
    }
}

OverlapResult SlopeCollider::checkOverlap(const Collider &cld_, int &highestPoint_) const
{
    int leftEdge = cld_.getLeftEdge();
    int rightEdge = cld_.getRightEdge();
    int topEdge = cld_.getTopEdge();
    int bottomEdge = cld_.getBottomEdge();

    if (m_topAngleCoef > 0)
    {
        int highesty = 0.0f;
        OverlapResult res = OverlapResult::NONE;

        if (leftEdge >= m_points[0].x && rightEdge <= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(leftEdge);
        }
        else if (leftEdge <= m_points[0].x && rightEdge >= m_points[0].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = m_points[0].y;
        }
        else if (leftEdge <= m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(leftEdge);
        }
        else
            return OverlapResult::NONE;
        
        highestPoint_ = highesty;
        if (bottomEdge < highesty || topEdge > m_points[2].y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    else if (m_topAngleCoef < 0)
    {
        int highesty = 0.0f;
        OverlapResult res = OverlapResult::NONE;

        if (leftEdge >= m_points[0].x && rightEdge <= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(rightEdge);
        }
        else if (leftEdge <= m_points[1].x && rightEdge >= m_points[1].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = m_points[1].y;
        }
        else if (leftEdge <= m_points[0].x && rightEdge >= m_points[0].x)
        {
            res |= OverlapResult::OVERLAP_X;
            highesty = getHeightAt(rightEdge);
        }
        else
            return OverlapResult::NONE;
        
        highestPoint_ = highesty;
        if (bottomEdge < highesty || topEdge > m_points[2].y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    else
    {
        OverlapResult res = OverlapResult::NONE;

        if (!( (rightEdge < m_points[0].x) || (leftEdge > m_points[1].x) ))
        {
            highestPoint_ = m_points[0].y;
            res |= OverlapResult::OVERLAP_X;
        }

        if (!( (bottomEdge < m_points[0].y) || (topEdge > m_points[3].y) ))
            res |= OverlapResult::OVERLAP_Y;

        return res;
    }
}

int SlopeCollider::getOrientationDir() const
{
    return ValueToOrientationInt(-m_topAngleCoef);
}

int SlopeCollider::getHeightAt(int x) const
{
    return m_tlPos.y + m_topAngleCoef * (x - m_tlPos.x);
}

int SlopeCollider::getMostRightAt(const Collider &cld_) const
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

int SlopeCollider::getMostLeftAt(const Collider &cld_) const
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

bool SlopeCollider::containsPoint(const Vector2<int> &point_) const
{
    if (point_.x < m_points[0].x || point_.x > m_points[0].x)
        return false;

    auto highest = getHeightAt(point_.x);

    return point_.y >= highest && point_.y <= m_points[2].y;
}

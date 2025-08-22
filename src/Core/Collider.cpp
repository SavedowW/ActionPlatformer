#include "Collider.h"

SlopeCollider::SlopeCollider(const Vector2<int> &tlPos_, const Vector2<int> &size_, float topAngleCoef_) :
    m_tlPos(tlPos_),
    m_size(size_),
    m_topAngleCoef(std::min(topAngleCoef_, 1.0f))
{
    generatePoints();
}

SlopeCollider::SlopeCollider(const Quad<int> &vertices_)
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

void SlopeCollider::set(const Quad<int> &vertices_)
{
    m_points = vertices_;

    m_tlPos = m_points.tl;
    m_size = m_points.br - m_points.tl;
    m_topAngleCoef = static_cast<float>(m_points.tr.y - m_points.tl.y) / static_cast<float>(m_size.x);

    m_highestSlopePoint = std::min(m_points.tl.y, m_points.tr.y);
    m_lowestSlopePoint = std::max(m_points.tl.y, m_points.tr.y);
    m_hasSlope = (m_points.tr.y - m_points.tl.y) != 0;
    m_hasBox = (m_lowestSlopePoint - m_points.br.y) > 0;
}

void SlopeCollider::generatePoints()
{
    m_points.tl = m_tlPos;
    m_points.tr = m_tlPos + Vector2<int>{m_size.x - 1, int((m_size.x - 1) * m_topAngleCoef)};
    m_points.br = m_tlPos + m_size - Vector2{1, 1};
    m_points.bl = m_tlPos + Vector2{0, m_size.y - 1};

    m_highestSlopePoint = std::min(m_points.tl.y, m_points.tr.y);
    m_lowestSlopePoint = std::max(m_points.tl.y, m_points.tr.y);
    m_hasSlope = (m_points.tr.y - m_points.tl.y) != 0;
    m_hasBox = (m_lowestSlopePoint - m_points.br.y) > 0;
}

Flag<OverlapResult> SlopeCollider::checkOverlap(const Collider &cld_) const
{
    int tmp = 0;
    return checkOverlap(cld_, tmp);
   
}

Flag<OverlapResult> SlopeCollider::checkOverlap(const Collider &cld_, int &highestPoint_) const
{
    Flag<OverlapResult> res{OverlapResult::NONE};

    // Slope goes downward to the right
    if (m_topAngleCoef > 0)
    {
        int leftEdge = cld_.getLeftEdge();
        if (leftEdge >= m_points.tl.x && leftEdge <= m_points.tr.x)
        {
            res |= OverlapResult::OVERLAP_X;
            highestPoint_ = getHeightAt(leftEdge);
        }
        else if (leftEdge <= m_points.tl.x && cld_.getRightEdge() >= m_points.tl.x)
        {
            res |= OverlapResult::OVERLAP_X;
            highestPoint_ = m_points.tl.y;
        }
        else
            return OverlapResult::NONE;
        
        if (cld_.getBottomEdge() < highestPoint_ || cld_.getTopEdge() > m_points.br.y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    
    // Slope goes downward to the left
    if (m_topAngleCoef < 0)
    {
        int rightEdge = cld_.getRightEdge();
        if (rightEdge >= m_points.tl.x && rightEdge <= m_points.tr.x)
        {
            res |= OverlapResult::OVERLAP_X;
            highestPoint_ = getHeightAt(rightEdge);
        }
        else if (cld_.getLeftEdge() <= m_points.tr.x && rightEdge >= m_points.tr.x)
        {
            res |= OverlapResult::OVERLAP_X;
            highestPoint_ = m_points.tr.y;
        }
        else
            return OverlapResult::NONE;
        
        if (cld_.getBottomEdge() < highestPoint_ || cld_.getTopEdge() > m_points.br.y)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    
    // Flat surface
    
    if ( (cld_.getRightEdge() >= m_points.tl.x) && (cld_.getLeftEdge() <= m_points.tr.x) )
    {
        highestPoint_ = m_points.tl.y;
        res |= OverlapResult::OVERLAP_X;
    }

    if ( (cld_.getBottomEdge() >= m_points.tl.y) && (cld_.getTopEdge() <= m_points.bl.y) )
        res |= OverlapResult::OVERLAP_Y;

    return res;
   
}

int SlopeCollider::getOrientationDir() const
{
    return ValueToOrientationInt(-m_topAngleCoef);
}

int SlopeCollider::getHeightAt(int x) const
{
    return m_tlPos.y + static_cast<int>(m_topAngleCoef * (x - m_tlPos.x));
}

int SlopeCollider::getMostRightAt(const Collider &cld_) const
{
    if (m_topAngleCoef <= 0) // Flat or up to the right
    {
        return m_points.tr.x;
    }
    else // Goes down to the right
    {
        auto bot = cld_.getBottomEdge();
        if (bot < m_lowestSlopePoint)
        {
            return m_tlPos.x + static_cast<int>((bot - m_tlPos.y) * m_topAngleCoef);
        }
        else
            return m_points.tr.x;
    }
}

int SlopeCollider::getMostLeftAt(const Collider &cld_) const
{
    if (m_topAngleCoef >= 0) // Flat or down to the right
    {
        return m_points.tl.x;
    }
    else // Goes down to the right
    {
        auto bot = cld_.getBottomEdge();
        if (bot < m_lowestSlopePoint)
        {
            return m_tlPos.x + static_cast<int>((bot - m_tlPos.y) * m_topAngleCoef);
        }
        else
            return m_points.tl.x;
    }
}

bool SlopeCollider::containsPoint(const Vector2<int> &point_) const
{
    if (point_.x < m_points.tl.x || point_.x > m_points.tl.x)
        return false;

    auto highest = getHeightAt(point_.x);

    return point_.y >= highest && point_.y <= m_points.br.y;
}

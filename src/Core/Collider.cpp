#include "Collider.h"

SlopeCollider::SlopeCollider(const Vector2<int> &topLeft_, const Vector2<int> &topRight_, int bottomHeight_) :
    m_topLeft(topLeft_),
    m_topRight(topRight_),
    m_bottomHeight(bottomHeight_),
    m_topAngleCoef(static_cast<float>(m_topRight.y - m_topLeft.y) / static_cast<float>(m_topRight.x - m_topLeft.x))
{
    assert(m_topLeft.x < m_topRight.x);

    if (m_topLeft.y > m_topRight.y)
    {
        m_highestSlopePoint = m_topRight.y;
        m_lowestSlopePoint = m_topLeft.y;
    }
    else
    {
        m_highestSlopePoint = m_topLeft.y;
        m_lowestSlopePoint = m_topRight.y;
    }
}

void SlopeCollider::set(const Vector2<int> &topLeft_, const Vector2<int> &topRight_, int bottomHeight_)
{
    m_topLeft = topLeft_;
    m_topRight = topRight_;
    m_bottomHeight = bottomHeight_;
    m_topAngleCoef = static_cast<float>(m_topRight.y - m_topLeft.y) / static_cast<float>(m_topRight.x - m_topLeft.x);

    assert(m_topLeft.x < m_topRight.x);

    if (m_topLeft.y > m_topRight.y)
    {
        m_highestSlopePoint = m_topRight.y;
        m_lowestSlopePoint = m_topLeft.y;
    }
    else
    {
        m_highestSlopePoint = m_topLeft.y;
        m_lowestSlopePoint = m_topRight.y;
    }
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
        if (leftEdge >= m_topLeft.x && leftEdge <= m_topRight.x)
        {
            res |= OverlapResult::OVERLAP_X;
            highestPoint_ = getHeightAt(leftEdge);
        }
        else if (leftEdge <= m_topLeft.x && cld_.getRightEdge() >= m_topLeft.x)
        {
            res |= OverlapResult::OVERLAP_X;
            highestPoint_ = m_topLeft.y;
        }
        else
            return OverlapResult::NONE;
        
        if (cld_.getBottomEdge() < highestPoint_ || cld_.getTopEdge() > m_bottomHeight)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    
    // Slope goes downward to the left
    if (m_topAngleCoef < 0)
    {
        int rightEdge = cld_.getRightEdge();
        if (rightEdge >= m_topLeft.x && rightEdge <= m_topRight.x)
        {
            res |= OverlapResult::OVERLAP_X;
            highestPoint_ = getHeightAt(rightEdge);
        }
        else if (cld_.getLeftEdge() <= m_topRight.x && rightEdge >= m_topRight.x)
        {
            res |= OverlapResult::OVERLAP_X;
            highestPoint_ = m_topRight.y;
        }
        else
            return OverlapResult::NONE;
        
        if (cld_.getBottomEdge() < highestPoint_ || cld_.getTopEdge() > m_bottomHeight)
            return res;
        
        return OverlapResult::OVERLAP_BOTH;
    }
    
    // Flat surface
    
    if ( (cld_.getRightEdge() >= m_topLeft.x) && (cld_.getLeftEdge() <= m_topRight.x) )
    {
        highestPoint_ = m_topLeft.y;
        res |= OverlapResult::OVERLAP_X;
    }

    if ( (cld_.getBottomEdge() >= m_topLeft.y) && (cld_.getTopEdge() <= m_bottomHeight) )
        res |= OverlapResult::OVERLAP_Y;

    return res;
}

SlopeCollider SlopeCollider::movedBy(const Vector2<int> &offset_) const noexcept
{
    return {m_topLeft + offset_, m_topRight + offset_, m_bottomHeight + offset_.y};
}

int SlopeCollider::getHeightAt(int x_) const
{
    assert(x_ >= m_topLeft.x);
    assert(x_ <= m_topRight.x);
    return m_topLeft.y + ((m_topRight.y - m_topLeft.y) * (x_ - m_topLeft.x) / (m_topRight.x - m_topLeft.x));
}

/*
     ->
     ->
    |\ ->
    | \ ->
    |  \ ->
    |   \ ->
    |    \ ->
    |     \ ->
    |      |->
    |      |->
    |      |->
    --------
*/
int SlopeCollider::getMostRightAt(const Collider &cld_) const
{
    // Flat or up to the right
    if (m_topAngleCoef <= 0)
        return m_topRight.x;

    // Goes down to the right
    const auto bot = cld_.getBottomEdge();

    if (bot <= m_highestSlopePoint)
        return m_topLeft.x;

    if (bot >= m_lowestSlopePoint)
        return m_topRight.x;

    return m_topLeft.x + ((m_topRight.x - m_topLeft.x) * (bot - m_topLeft.y) / (m_lowestSlopePoint - m_highestSlopePoint));
}

int SlopeCollider::getMostLeftAt(const Collider &cld_) const
{
    // Flat or down to the right
    if (m_topAngleCoef >= 0)
        return m_topLeft.x;

    // Goes up to the right
    const auto bot = cld_.getBottomEdge();

    if (bot <= m_highestSlopePoint)
        return m_topRight.x;

    if (bot >= m_lowestSlopePoint)
        return m_topLeft.x;

    return m_topRight.x - ((m_topRight.x - m_topLeft.x) * (m_topLeft.y - bot) / (m_lowestSlopePoint - m_highestSlopePoint));
}


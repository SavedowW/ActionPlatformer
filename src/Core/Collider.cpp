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
            return m_points[1].x;
    }
}

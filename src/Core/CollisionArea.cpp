#include "CollisionArea.h"

void CollisionArea::addStaticCollider(const SlopeCollider &cld_)
{
    if (cld_.m_isObstacle)
        m_obstacles.push_back(m_staticCollisionMap.size());

    m_staticCollisionMap.push_back(cld_);
}

bool CollisionArea::getHighestVerticalMagnetCoord(const Collider &cld_, float &coord_, bool isFallingThrough_) const
{
    float baseCoord = coord_;
    float bot = cld_.y + cld_.h;
    bool isFound = false;
    for (const auto &cld : m_staticCollisionMap)
    {
        if (isFallingThrough_ && cld.m_isObstacle)
            continue;

        auto horOverlap = cld.getHorizontalOverlap(cld_);
        if (horOverlap)
        {
            auto height = cld.getTopHeight(cld_, horOverlap);
            if (height > baseCoord && (!isFound || height < coord_))
            {
                coord_ = height;
                isFound = true;
            }
        }
    }

    return isFound;
}

bool CollisionArea::checkPlayerTouchingObstacles(const Collider &playerPb_) const
{
    float dumped = 0.0f;
    for (const auto &i : m_obstacles)
    {
        if (m_staticCollisionMap[i].getFullCollisionWith<false, false>(playerPb_, dumped))
            return true;
    }

    return false;
}

#include "CollisionArea.h"

void CollisionArea::addStaticCollider(const SlopeCollider &cld_)
{
    m_staticCollisionMap.push_back(cld_);
}

bool CollisionArea::getHighestVerticalMagnetCoord(const Collider &cld_, float &coord_) const
{
    float bot = cld_.y + cld_.h;
    bool isFound = false;
    for (const auto &cld : m_staticCollisionMap)
    {
        if (bot > cld.m_lowestSlopePoint)
            continue;

        auto horOverlap = cld.getHorizontalOverlap(cld_);
        if (horOverlap)
        {
            auto height = cld.getTopHeight(cld_, horOverlap);
            if (!isFound || height < coord_)
            {
                coord_ = height;
                isFound = true;
            }
        }
    }

    return isFound;
}

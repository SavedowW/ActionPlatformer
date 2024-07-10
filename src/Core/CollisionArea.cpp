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
        if (bot > cld.m_lowestSlopePoint || cld.m_isDisabled)
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

bool CollisionArea::disableStaticCollider(SlopeCollider &cld_)
{
    if (!cld_.m_isObstacle || cld_.m_isDisabled)
        return false;

    cld_.m_isDisabled = true;
    m_disabledColliders.push_back(&cld_);
    return true;
}

void CollisionArea::recoverColliders(const Collider &playerPb_)
{
    float dumped = 0.0f;
    for (int i = 0; i < m_disabledColliders.size(); )
    {
        if (!m_disabledColliders[i]->getFullCollisionWith<true, true, false>(playerPb_, dumped))
        {
            m_disabledColliders[i]->m_isDisabled = false;
            m_disabledColliders.erase(m_disabledColliders.begin() + i);
        }
        else
            ++i;
    }
}

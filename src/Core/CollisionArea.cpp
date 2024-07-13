#include "CollisionArea.h"
#include "PlayableCharacter.h"

void CollisionArea::addStaticCollider(const SlopeCollider &cld_)
{
    int insertId = (cld_.m_hasSlope ? m_nextSlope++ : m_staticCollisionMap.size());
    m_staticCollisionMap.insert(m_staticCollisionMap.begin() + insertId, cld_);

    if (cld_.m_obstacleId)
        m_obstacles.push_back(insertId);

}

bool CollisionArea::getHighestVerticalMagnetCoord(const Collider &cld_, float &coord_, const PlayableCharacter &char_) const
{
    float baseCoord = coord_;
    float bot = cld_.y + cld_.h;
    bool isFound = false;
    for (const auto &cld : m_staticCollisionMap)
    {
        if (cld.m_obstacleId && char_.checkIgnoringObstacle(cld.m_obstacleId))
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

std::set<int> CollisionArea::getPlayerTouchingObstacles(const Collider &playerPb_) const
{
    std::set<int> obstacleIds;
    float dumped = 0.0f;
    for (const auto &i : m_obstacles)
    {
        if (obstacleIds.contains(m_staticCollisionMap[i].m_obstacleId))
            continue;

        if (m_staticCollisionMap[i].getFullCollisionWith<false, false>(playerPb_, dumped))
            obstacleIds.insert(m_staticCollisionMap[i].m_obstacleId);
    }

    return obstacleIds;
}

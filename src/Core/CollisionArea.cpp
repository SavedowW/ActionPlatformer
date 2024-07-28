#include "CollisionArea.h"

bool CollisionArea::isAreaFree(const Collider &cld_, bool considerObstacles_)
{
    float dumped = 0;
    for (const auto &scld : m_staticCollisionMap)
    {
        if (!considerObstacles_ && scld.m_obstacleId)
            continue;
        auto colres = scld.getFullCollisionWith(cld_, dumped);
        if ((colres & utils::OverlapResult::OVERLAP_X) && (colres & utils::OverlapResult::OVERLAP_Y))
            return false;
    }

    return true;
}

const Trigger *CollisionArea::getOverlappedTrigger(const Collider &cld_, Trigger::Tag tag_) const
{
    for (const auto &trg : m_triggers)
    {
        if ((trg.m_tag & tag_) == tag_ && ((trg.checkCollisionWith(cld_) & utils::OverlapResult::BOTH_OOT) == utils::OverlapResult::BOTH_OVERLAP))
            return &trg;
    }

    return nullptr;
}

std::set<int> CollisionArea::getPlayerTouchingObstacles(const Collider &playerPb_) const
{
    std::set<int> obstacleIds;
    float dumped = 0.0f;
    for (const auto &i : m_obstacles)
    {
        if (obstacleIds.contains(m_staticCollisionMap[i].m_obstacleId))
            continue;

        if (!!(m_staticCollisionMap[i].getFullCollisionWith(playerPb_, dumped) & utils::OverlapResult::BOTH_OOT))
            obstacleIds.insert(m_staticCollisionMap[i].m_obstacleId);
    }

    return obstacleIds;
}

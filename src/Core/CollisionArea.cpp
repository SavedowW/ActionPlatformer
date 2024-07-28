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

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

void CollisionArea::finalize()
{
    float topOffset = 50.0f;
    float botOffset = 50.0f;
    for (const auto& cld_ : m_staticCollisionMap)
    {
        std::pair<Vector2<float>, Vector2<float>> leftSide = {cld_.m_points[0], cld_.m_points[3] - cld_.m_points[0]};
        std::pair<Vector2<float>, Vector2<float>> rightSide = {cld_.m_points[1], cld_.m_points[2] - cld_.m_points[1]};

        if (leftSide.second.y >= topOffset + botOffset)
        {
            Trigger trgarea{leftSide.first.x - 1, leftSide.first.y + topOffset, 1.0f, leftSide.second.y - topOffset - botOffset};
            trgarea |= Trigger::Tag::ClingArea | Trigger::Tag::LEFT;
            if (isAreaFree(trgarea, false))
                m_triggers.push_back(trgarea);
        }

        if (rightSide.second.y >= topOffset + botOffset)
        {
            Trigger trgarea{rightSide.first.x, rightSide.first.y + topOffset, 1.0f, rightSide.second.y - topOffset - botOffset};
            trgarea |= Trigger::Tag::ClingArea | Trigger::Tag::RIGHT;
            if (isAreaFree(trgarea, false))
                m_triggers.push_back(trgarea);
        }
    }
}

bool CollisionArea::isAreaFree(const Collider &cld_, bool considerObstacles_)
{
    float dumped = 0;
    for (const auto &scld : m_staticCollisionMap)
    {
        if (!considerObstacles_ && scld.m_obstacleId)
            continue;
        if (scld.getFullCollisionWith<true, true, false>(cld_, dumped))
            return false;
    }

    return true;
}

const Trigger *CollisionArea::getOverlappedTrigger(const Collider &cld_, Trigger::Tag tag_) const
{
    for (const auto &trg : m_triggers)
    {
        if ((trg.m_tag & tag_) == tag_ && trg.checkCollisionWith<true, true>(cld_))
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

        if (m_staticCollisionMap[i].getFullCollisionWith<false, false>(playerPb_, dumped))
            obstacleIds.insert(m_staticCollisionMap[i].m_obstacleId);
    }

    return obstacleIds;
}

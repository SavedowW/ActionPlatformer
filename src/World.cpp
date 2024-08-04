#include "World.h"
#include "CoreComponents.h"

World::World(entt::registry &reg_) :
    m_registry(reg_)
{
}

bool World::isAreaFree(const Collider &cld_, bool considerObstacles_)
{
    auto cldview = m_registry.view<ComponentStaticCollider>();
    float dumped = 0;

    for (auto [idx, cld] : cldview.each())
    {
        if (m_registry.all_of<ComponentObstacle>(idx) && !considerObstacles_)
            continue;

        auto colres = cld.m_collider.getFullCollisionWith(cld_, dumped);
        if (!!(colres & utils::OverlapResult::BOTH_OVERLAP))
            return false;
    }

    return true;
}

EntityAnywhere World::getOverlappedTrigger(const Collider &cld_, Trigger::Tag tag_) const
{
    auto trgview = m_registry.view<ComponentTrigger>();

    for (auto [idx, trg] : trgview.each())
    {
        if ((trg.m_trigger.m_tag & tag_) == tag_ && ((trg.m_trigger.checkCollisionWith(cld_) & utils::OverlapResult::BOTH_OOT) == utils::OverlapResult::BOTH_OVERLAP))
            return {&m_registry, idx};
    }

    return {nullptr};
}

bool World::touchingWallAt(ORIENTATION checkSide_, const Vector2<float> &pos_)
{
    auto cldview = m_registry.view<ComponentStaticCollider>();
    for (auto [idx, cld] : cldview.each())
    {
        if (m_registry.all_of<ComponentObstacle>(idx))
            continue;

        if (checkSide_ == ORIENTATION::LEFT)
        {
            if (cld.m_collider.m_points[0].x == pos_.x && cld.m_collider.m_points[0].y < pos_.y && cld.m_collider.m_points[3].y > pos_.y)
                return true;
        }
        else if (checkSide_ == ORIENTATION::RIGHT)
        {
            if (cld.m_collider.m_points[1].x == pos_.x && cld.m_collider.m_points[1].y < pos_.y && cld.m_collider.m_points[2].y > pos_.y)
                return true;
        }
    }

    return false;
}

bool World::touchingGround(const Collider &cld_, ComponentObstacleFallthrough &fallthrough_)
{
    auto cldview = m_registry.view<ComponentStaticCollider>();
    for (auto [idx, cld] : cldview.each())
    {
        if (m_registry.all_of<ComponentObstacle>(idx) && fallthrough_.checkIgnoringObstacle(m_registry.get<ComponentObstacle>(idx).m_obstacleId))
            continue;

        float dump = 0;
        auto cldres = cld.m_collider.getFullCollisionWith(cld_, dump);
        if ((cldres & utils::OverlapResult::OVERLAP_X) && (cldres & (utils::OverlapResult::TOUCH_MIN1_MAX2 << 6)))
            return true;
    }
    return false;
}

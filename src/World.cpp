#include "World.h"
#include "CoreComponents.h"

World::World(entt::registry &reg_, Camera &cam_, ParticleSystem &partsys_, NavSystem &navsys_) :
    m_registry(reg_),
    m_cam(cam_),
    m_partsys(partsys_),
    m_navsys(navsys_)
{
}

bool World::isAreaFree(const Collider &cld_, bool considerObstacles_) const
{
    auto cldview = m_registry.view<ComponentStaticCollider>();

    for (auto [idx, cld] : cldview.each())
    {
        if (!cld.m_isEnabled || cld.m_obstacleId && !considerObstacles_)
            continue;

        auto colres = cld.m_collider.checkOverlap(cld_);
        if (checkCollision(colres, CollisionResult::OVERLAP_BOTH))
            return false;
    }

    return true;
}

bool World::isOverlappingObstacle(const Collider &cld_) const
{
    auto cldview = m_registry.view<ComponentStaticCollider>();

    for (auto [idx, cld] : cldview.each())
    {
        if (cld.m_isEnabled && cld.m_obstacleId)
        {
            auto colres = cld.m_collider.checkOverlap(cld_);
            if (checkCollision(colres, CollisionResult::OVERLAP_BOTH))
                return true;
        }
    }

    return false;
}

EntityAnywhere World::getOverlappedTrigger(const Collider &cld_, Trigger::Tag tag_) const
{
    auto trgview = m_registry.view<ComponentTrigger>();

    for (auto [idx, trg] : trgview.each())
    {
        if ((trg.m_trigger.m_tag & tag_) == tag_ && checkCollision(trg.m_trigger.checkOverlap(cld_), CollisionResult::OVERLAP_BOTH))
            return {&m_registry, idx};
    }

    return {nullptr};
}

bool World::touchingWallAt(ORIENTATION checkSide_, const Vector2<float> &pos_) const
{
    auto cldview = m_registry.view<ComponentStaticCollider>();
    for (auto [idx, cld] : cldview.each())
    {
        if (!cld.m_isEnabled || cld.m_obstacleId)
            continue;

        if (checkSide_ == ORIENTATION::LEFT)
        {
            if (abs(cld.m_collider.m_points[0].x - pos_.x) <= 1.0f && cld.m_collider.m_points[0].y < pos_.y && cld.m_collider.m_points[3].y > pos_.y)
                return true;
        }
        else if (checkSide_ == ORIENTATION::RIGHT)
        {
            if (abs(cld.m_collider.m_points[1].x - pos_.x) <= 1.0f && cld.m_collider.m_points[1].y < pos_.y && cld.m_collider.m_points[2].y > pos_.y)
                return true;
        }
    }

    return false;
}

Camera &World::getCamera()
{
    return m_cam;
}

ParticleSystem &World::getParticleSys()
{
    return m_partsys;
}

NavSystem &World::getNavsys()
{
    return m_navsys;
}

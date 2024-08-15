#include "DynamicColliderSystem.h"
#include "CoreComponents.h"

DynamicColliderSystem::DynamicColliderSystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void DynamicColliderSystem::update()
{
    auto clds = m_reg.view<ComponentStaticCollider, SwitchCollider>();

    for (auto [idx, cld, swc] : clds.each())
    {
        auto res = swc.updateTimer();
        if (res == true)
            std::cout << "";
        if (res != cld.m_isEnabled)
        {
            //std::cout << "Trying to switch...\n";
            bool overlapping = (cld.m_obstacleId ? isObstacleOverlappingWithDynamic(cld.m_collider, cld.m_obstacleId) : isOverlappingWithDynamic(cld.m_collider));
            if (!overlapping)
                cld.m_isEnabled = res;
        }
    }
}

bool DynamicColliderSystem::isOverlappingWithDynamic(const SlopeCollider &cld_)
{
    auto dynamics = m_reg.view<ComponentTransform, ComponentPhysical>();
    for (auto [idx, trans, phys] : dynamics.each())
    {
        auto pb = phys.m_pushbox + trans.m_pos;
        float dump = 0.0f;
        auto res = cld_.getFullCollisionWith(pb, dump);
        if ((res & utils::OverlapResult::OVERLAP_X) && (res & utils::OverlapResult::OVERLAP_Y))
            return true;
    }

    return false;
}

bool DynamicColliderSystem::isObstacleOverlappingWithDynamic(const SlopeCollider &cld_, int obstacleId_)
{
    auto dynamics = m_reg.view<ComponentTransform, ComponentPhysical>();
    for (auto [idx, trans, phys] : dynamics.each())
    {
        auto pb = phys.m_pushbox + trans.m_pos;
        float dump = 0.0f;
        auto res = cld_.getFullCollisionWith(pb, dump);
        if ((res & utils::OverlapResult::OVERLAP_X) && (res & utils::OverlapResult::OVERLAP_Y))
        {
            if (!m_reg.all_of<ComponentObstacleFallthrough>(idx))
                return true;
            else
                m_reg.get<ComponentObstacleFallthrough>(idx).setIgnoreObstacle(obstacleId_);
        }
    }

    return false;
}

#ifndef DYNAMIC_COLLIDER_SYSTEM_H_
#define DYNAMIC_COLLIDER_SYSTEM_H_
#include "World.h"
#include <entt/entt.hpp>

struct DynamicColliderSystem
{
    DynamicColliderSystem(entt::registry &reg_);

    void updateSwitchingColliders();
    void updateMovingColliders();

    void proceedMovingCollider(ComponentTransform &trans_, ComponentStaticCollider &scld_, MoveCollider2Points &twop_);

    bool isOverlappingWithDynamic(const SlopeCollider &cld_);
    bool isObstacleOverlappingWithDynamic(const SlopeCollider &cld_, int obstacleId_);

    entt::registry &m_reg;
};

#endif

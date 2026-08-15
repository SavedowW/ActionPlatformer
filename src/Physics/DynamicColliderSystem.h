#pragma once
#include "ColliderRouting.h"
#include "Core/CoreComponents.h"
#include <entt/entt.hpp>

struct DynamicColliderSystem
{
    DynamicColliderSystem(entt::registry &reg_);

    void updateMovingColliders();
    
private:
    void solveRouteIter(MoveCollider2Points &m2p_, ColliderRoutingIterator &iter_);

    void proceedMovingCollider(entt::entity cid_, ComponentTransform &trans_, ComponentStaticCollider &scld_, MoveCollider2Points &twop_);
    
    bool isOverlappingWithDynamic(const SlopeCollider &cld_);
    bool isObstacleOverlappingWithDynamic(entt::entity cid_, const SlopeCollider &cld_, int obstacleId_);

    /**
     *  cid_ - collider ID
     */
    void moveColliderAt(entt::entity cid_, ComponentTransform &trans_, ComponentStaticCollider &scld_, const Vector2<int> &newtl_);

    entt::registry &m_reg;
};

#pragma once
#include "ColliderRouting.h"
#include "Core/CoreComponents.h"
#include "PhysicsSystem.h"
#include <entt/entt.hpp>

// Dynamic Collider System
namespace DCS
{
    using MovingCollidersView = decltype(entt::registry{}.view<ComponentTransform, ComponentStaticCollider, MoveCollider2Points>());
    using DynamicsView = decltype(entt::registry{}.view<ComponentTransform, ComponentPhysical, WorldPosition, ComponentObstacleFallthrough>());
}

class DynamicCollidersHandler
{
public:
    DynamicCollidersHandler(const DCS::MovingCollidersView &movingCollidersView_, const DCS::DynamicsView &dynamicsView_, const CollidersView &collidersView_);

    void Handle();

protected:
    /**
     *  cid_ - collider ID
     *  Purpose:
     *   - Set `trans_` position to the new position
     *   - Set `scld_` resolved collider appropriately
     *   - For each dynamic entity:
     *     - Move using handler from physics system 
     *     - Update phys.enforcedOffset, phys.pushedOffset
     */
    void moveColliderAt(entt::entity cid_, ComponentTransform &trans_, ComponentStaticCollider &scld_, const Vector2<int> &newtl_);

    PhysicsEntityHandler &getDynamicHandler(entt::entity idx_, ComponentTransform &trans_, 
        ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, WorldPosition &worldPos_);

private:
    DCS::MovingCollidersView m_movingCollidersView;
    DCS::DynamicsView m_dynamicsView;
    CollidersView m_collidersView;

    std::unordered_map<entt::entity, PhysicsEntityHandler> m_handlers;
};

class DynamicColliderSystem
{
public:
    DynamicColliderSystem(entt::registry &reg_);

    void updateMovingColliders();
    
private:
    static void solveRouteIter(MoveCollider2Points &m2p_, ColliderRoutingIterator &iter_);

    entt::registry &m_reg;
};

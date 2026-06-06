#pragma once
#include "Core/CoreComponents.h"
#include <entt/entt.hpp>

using CollidersView = decltype(entt::registry{}.view<ComponentStaticCollider>());

class PhysicsEntityHandler
{
public:
    PhysicsEntityHandler(const CollidersView &cld_, ComponentTransform &trans_, 
        ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, WorldPosition &worldPos_);

    void moveRight(int offset_);
    void moveLeft(int offset_);
    void moveDown(int offset_);
    void moveUp(int offset_);
    
    void magnet();

    /**
     *  Update:
     *   PhysicalEvents
     *    (currently none)
     *   ComponentPhysical:
     *    onSlopeWithAngle
     *    onGround
     *   ComponentObstacleFallthrough:
     *    ignoredObstacles
     */
    void discoverPosition();
    
private:
    const SlopeCollider *getHighestVerticalMagnetCoord(int &coord_);

    const CollidersView &m_cld;

    ComponentTransform &m_trans;
    ComponentPhysical &m_phys;
    const Collider m_pushbox;
    ComponentObstacleFallthrough &m_obsFallthrough;
    WorldPosition &m_worldPos;

    bool m_requireMagnet = false;

    static const float VerticalOffsetLimitMul;
};

class PhysicsSystem
{
public:
    PhysicsSystem(entt::registry &reg_, Vector2<int> levelSize_);

    void prepHitstop();
    void prepEntities();
    void updatePhysics();

private:
    static void proceedEntity(const CollidersView &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, WorldPosition &worldPos_);
    
    entt::registry &m_reg;
    const Vector2<int> m_levelSize;
};

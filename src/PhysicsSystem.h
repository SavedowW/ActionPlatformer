#ifndef PHYSICS_SYSTEM_H_
#define PHYSICS_SYSTEM_H_
#include "CoreComponents.h"
#include "StateMachine.h"
#include <entt/entt.hpp>

struct PhysicsSystem
{
    PhysicsSystem(entt::registry &reg_, Vector2<float> levelSize_);

    void updateSMs();
    void updatePhysics();
    void updatePhysicalEvents();

    void proceedEntity(auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, PhysicalEvents &ev_);
    void proceedEntity(auto &clds_, ComponentTransform &trans_, ComponentParticlePhysics &phys_);
    
    bool magnetEntity(auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    std::pair<bool, const SlopeCollider*> getHighestVerticalMagnetCoord(auto &clds_, const Collider &cld_, float &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_);

    void resetEntityObstacles(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    std::set<int> getTouchedObstacles(const Collider &pb_);

    entt::registry &m_reg;
    Vector2<float> m_levelSize;
};


#endif

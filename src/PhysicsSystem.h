#ifndef PHYSICS_SYSTEM_H_
#define PHYSICS_SYSTEM_H_
#include "CoreComponents.h"
#include "StateMachine.h"
#include <entt/entt.hpp>

struct PhysicsSystem
{
    PhysicsSystem(entt::registry &reg_, Vector2<float> levelSize_);

    void update();

    void proceedEntity(auto &clds_, entt::entity idx_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, StateMachine *sm_);
    
    bool magnetEntity(auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    bool getHighestVerticalMagnetCoord(auto &clds_, const Collider &cld_, float &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_);

    void resetEntityObstacles(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    std::set<int> getTouchedObstacles(const Collider &pb_);

    entt::registry &m_reg;
    Vector2<float> m_levelSize;
};


#endif
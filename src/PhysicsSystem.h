#ifndef PHYSICS_SYSTEM_H_
#define PHYSICS_SYSTEM_H_
#include "Core/CoreComponents.h"
#include <entt/entt.hpp>

struct PhysicsSystem
{
    PhysicsSystem(entt::registry &reg_, Vector2<int> levelSize_);

    void prepHitstop();
    void prepEntities();
    void updateSMs();
    void updatePhysics(const double &partOfSecond_);
    void updatePhysicalEvents();
    void updateOverlappedObstacles(const Time::NS &frameTime_);

    /*
        Attempt to move entity in a direction with offset if it doesn't collide with anything, unchanged otherwise
        True if succeeded
    */
    bool attemptOffsetDown(const auto &clds_, const Vector2<float> &originalPos_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, unsigned int offset_,
        bool noLanding_, float &touchedSlope_, entt::entity &onGround_);
    bool attemptOffsetUp(const auto &clds_, const Vector2<float> &originalPos_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, unsigned int offset_);
    bool attemptOffsetHorizontal(const auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, int offset_,
        int originalY_, unsigned int maxYOffset_, int naturalYOffset_, float &touchedSlope_, entt::entity &onGround_);

    void proceedEntity(const auto &clds_, const entt::entity &idx_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, PhysicalEvents &ev_, const double &partOfSecond_);
    void proceedEntity(ComponentTransform &trans_, ComponentParticlePhysics &phys_, const double &partOfSecond_);
    
    bool magnetEntity(const auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, const ComponentObstacleFallthrough &obsFallthrough_);
    std::pair<entt::entity, const SlopeCollider*> getHighestVerticalMagnetCoord(const auto &clds_, const Collider &cld_, int &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_);

    // Evil, iterates over all entities for a selected entity
    bool isInsidePushbox(const Collider &pb_, const entt::entity &idx_);

    void resetEntityObstacles(const ComponentTransform &trans_, const ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, const auto &clds_);
    void updateTouchedObstacles(const Collider &pb_, ComponentObstacleFallthrough &obsFallthrough_, const auto &clds_);

    entt::registry &m_reg;
    const Vector2<int> m_levelSize;
};


#endif

#ifndef WORLD
#define WORLD
#include "Trigger.h"
#include "CoreComponents.h"
#include "ParticleSystem.h"
#include "NavSystem.h"
#include "Camera.h"
#include <entt/entt.hpp>

struct EntityAnywhere
{
    entt::registry *reg = nullptr;
    entt::entity idx = entt::null;
};

// Interface used by entities to access world data (create particles, run collision checks, cause camera shake, access navigation system)
class World
{
public:
    World(entt::registry &reg_, Camera &cam_, ParticleSystem &partsys_, NavSystem &navsys_);
    bool isAreaFree(const Collider &cld_, bool considerObstacles_) const;
    bool isOverlappingObstacle(const Collider &cld_) const;
    EntityAnywhere getOverlappedTrigger(const Collider &cld_, Trigger::Tag tag_) const;
    entt::entity isWallAt(const ORIENTATION checkSide_, const Vector2<int> &pos_) const;

    ParticleSystem &getParticleSys();
    
    // TODO: Move to renderer
    Camera &getCamera();

    // TODO: To navigatable component
    NavSystem &getNavsys();

private:
    entt::registry &m_registry;
    Camera &m_cam;
    ParticleSystem &m_partsys;
    NavSystem &m_navsys;

};

#endif
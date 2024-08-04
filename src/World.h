#ifndef WORLD
#define WORLD
#include "Trigger.h"
#include "CoreComponents.h"
#include <entt/entt.hpp>

struct EntityAnywhere
{
    entt::registry *reg;
    entt::entity idx;
};

class World
{
public:
    World(entt::registry &reg_);
    bool isAreaFree(const Collider &cld_, bool considerObstacles_);
    EntityAnywhere getOverlappedTrigger(const Collider &cld_, Trigger::Tag tag_) const;
    bool touchingWallAt(ORIENTATION checkSide_, const Vector2<float> &pos_);
    bool touchingGround(const Collider &cld_, ComponentObstacleFallthrough &fallthrough_);

private:
    entt::registry &m_registry;

};

#endif
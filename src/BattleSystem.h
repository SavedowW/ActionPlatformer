#ifndef BATTLE_SYSTEM_H_
#define BATTLE_SYSTEM_H_
#include "Core/Camera.h"
#include "Core/FixedQueue.hpp"
#include <entt/entt.hpp>

struct BattleActor;
class StateMachine;
struct ComponentTransform;
struct HitboxGroup;

struct ActorDescr
{
    entt::entity &m_id;
    BattleActor &m_actor;
    StateMachine &m_sm;
    ComponentTransform &m_trans;
};

struct BattleSystem
{
    BattleSystem(entt::registry &reg_, Camera &cam_);

    void update();
    void handleAttacks();
    void debugDraw();
    void applyHit(ActorDescr attacker_, ActorDescr victim_, const HitboxGroup &hit_);

private:
    entt::registry &m_reg;
    Camera &m_cam;

    std::set<uint32_t> m_presentHits;
    FixedQueue<Vector2<float>, 5> m_appliedHits;

};

#endif

#ifndef ENEMY_SYSTEM_H_
#define ENEMY_SYSTEM_H_
#include "Core/NavSystem.h"
#include "Core/Camera.h"
#include "ParticleSystem.h"
#include <entt/entt.hpp>

struct EnemySystem
{
    EnemySystem(entt::registry &reg_, NavSystem &navsys_, Camera &cam_, ParticleSystem &partsys_);

    entt::entity makeEnemy();
    void update();
    void setPlayerId(const entt::entity &playerId_);

    entt::registry &m_reg;
    entt::entity m_playerId;
    AnimationManager &m_animManager;
    NavSystem &m_navsys;
    ParticleSystem &m_partsys;
    Camera &m_cam;
};

#endif

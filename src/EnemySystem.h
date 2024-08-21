#ifndef ENEMY_SYSTEM_H_
#define ENEMY_SYSTEM_H_
#include "Application.h"
#include "NavSystem.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include <entt/entt.hpp>

struct EnemySystem
{
    EnemySystem(entt::registry &reg_, Application &app_, NavSystem &navsys_, Camera &cam_, ParticleSystem &partsys_);

    void makeEnemy();
    void update();

    entt::registry &m_reg;
    entt::entity m_playerId;
    AnimationManager &m_animManager;
    NavSystem &m_navsys;
    ParticleSystem &m_partsys;
    Camera &m_cam;
};

#endif

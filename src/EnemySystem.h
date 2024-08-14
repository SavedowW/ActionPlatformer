#ifndef ENEMY_SYSTEM_H_
#define ENEMY_SYSTEM_H_
#include "Application.h"
#include <entt/entt.hpp>

struct EnemySystem
{
    EnemySystem(entt::registry &reg_, Application &app_);

    void makeEnemy();
    void update();

    entt::registry &m_reg;
    entt::entity m_playerId;
    AnimationManager &m_animManager;
};

#endif

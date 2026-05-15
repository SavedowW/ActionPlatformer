#pragma once
#include "Core/NavSystem.h"
#include "Core/Camera.h"
#include "ParticleSystem.h"
#include "PlayerSystem.h"
#include <entt/entt.hpp>

class EnemySystem
{
public:
    EnemySystem(entt::registry &reg_, NavSystem &navsys_, Camera &cam_, ParticleSystem &partsys_, const PlayerSystem &playerSystem_);

    entt::entity makeEnemy();
    void update();

private:
    entt::registry &m_reg;
    AnimationManager &m_animManager;
    const PlayerSystem &m_playersys;
    NavSystem &m_navsys;
    ParticleSystem &m_partsys;
    Camera &m_cam;
};

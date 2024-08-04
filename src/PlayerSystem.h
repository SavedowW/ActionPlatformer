#ifndef PLAYER_SYSTEM_H_
#define PLAYER_SYSTEM_H_
#include "PlayableCharacter.h"
#include "Application.h"
#include <entt/entt.hpp>

struct PlayerSystem
{
    PlayerSystem(entt::registry &reg_, Application &app_);

    void setup();
    void update();

    entt::registry &m_reg;
    AnimationManager &m_animManager;
};

#endif
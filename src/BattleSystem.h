#ifndef BATTLE_SYSTEM_H_
#define BATTLE_SYSTEM_H_
#include "Application.h"
#include <entt/entt.hpp>

struct BattleSystem
{
    BattleSystem(entt::registry &reg_, Application &app_);

    void update();

    entt::registry &m_reg;
    Application &m_app;
};

#endif

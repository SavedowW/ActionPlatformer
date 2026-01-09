#ifndef AI_SYSTEM_H_
#define AI_SYSTEM_H_
#include "CommonAI.h"
#include "Core/Application.h"
#include <entt/entt.hpp>

struct AISystem
{
public:
    AISystem(entt::registry &reg_);

    void update();

    entt::entity m_playerId;

private:
    entt::registry &m_reg;
};

#endif
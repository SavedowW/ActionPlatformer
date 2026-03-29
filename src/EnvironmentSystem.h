#pragma once
#include "Core/Application.h"
#include <entt/entt.hpp>

class EnvironmentSystem
{
public:
    EnvironmentSystem(entt::registry &reg_);

    void update(entt::entity playerId_);

private:
    entt::registry &m_reg;
};

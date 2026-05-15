#pragma once
#include "Core/Application.h"
#include <entt/entt.hpp>

class EnvironmentSystem
{
public:
    EnvironmentSystem(entt::registry &reg_);

    void update();

private:
    entt::registry &m_reg;
};

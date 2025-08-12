#ifndef ENVIRONMENT_SYSTEM_H_
#define ENVIRONMENT_SYSTEM_H_
#include "Application.h"
#include <entt/entt.hpp>

class EnvironmentSystem
{
public:
    EnvironmentSystem(entt::registry &reg_);

    void update(entt::entity playerId_);

private:
    entt::registry &m_reg;
};

#endif

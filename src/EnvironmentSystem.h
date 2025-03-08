#ifndef ENVIRONMENT_SYSTEM_H_
#define ENVIRONMENT_SYSTEM_H_
#include "Application.h"
#include <entt/entt.hpp>

class EnvironmentSystem
{
public:
    EnvironmentSystem(Application &app_, entt::registry &reg_);

    void makeGrassTop(const Vector2<float> &pos_);

private:
    Application &m_app;
    entt::registry &m_reg;
};

#endif

#ifndef ENVIRONMENT_SYSTEM_H_
#define ENVIRONMENT_SYSTEM_H_
#include "Application.h"
#include <entt/entt.hpp>
#include "ObjectList.h"

class EnvironmentSystem
{
public:
    EnvironmentSystem(Application &app_, entt::registry &reg_);

    void makeObject(ObjectClass objClass_, const Vector2<int> &pos_, bool visible_, int layer_);
    void makeGrassTop(const Vector2<int> &pos_, bool visible_, int layer_);

    void update(entt::entity playerId_);

private:
    Application &m_app;
    entt::registry &m_reg;
};

#endif

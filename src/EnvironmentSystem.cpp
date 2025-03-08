#include "EnvironmentSystem.h"

EnvironmentSystem::EnvironmentSystem(Application &app_, entt::registry &reg_) :
    m_app(app_),
    m_reg(reg_)
{
}

void EnvironmentSystem::makeGrassTop(const Vector2<float> &pos_)
{
    //auto enemyId = m_reg.create();
    //m_reg.emplace<ComponentTransform>(enemyId, Vector2{170.0f, 300.0f}, ORIENTATION::RIGHT);
}

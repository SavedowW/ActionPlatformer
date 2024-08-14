#include "AISystem.h"

AISystem::AISystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void AISystem::update()
{
    auto view = m_reg.view<ComponentAI>();

    for (auto [idx, ai] : view.each())
    {
        ai.m_sm.update({&m_reg, idx}, 0);
    }
}

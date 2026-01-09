#include "AISystem.h"
#include "Core/Profile.h"

AISystem::AISystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void AISystem::update()
{
    PROFILE_FUNCTION;

    auto view = m_reg.view<ComponentAI>();

    for (auto [idx, ai] : view.each())
    {
        ai.m_sm.update({&m_reg, idx}, 0);
    }

    /* TODO: notably faster in release build, but harder to debug even with seq, might add debug flags to enable parallel execution
    auto iteratable = view.each();
    std::for_each(std::execution::par, iteratable.begin(), iteratable.end(), [&](auto inp)
    {
        auto [idx, ai] = inp;
        ai.m_sm.update({&m_reg, idx}, 0);
    });
    */
}

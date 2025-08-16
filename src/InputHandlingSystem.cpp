#include "InputHandlingSystem.h"
#include "InputResolver.h"
#include "Profile.h"

InputHandlingSystem::InputHandlingSystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void InputHandlingSystem::update()
{
    PROFILE_FUNCTION;

    auto view = m_reg.view<InputResolver>();
    for (auto [idx, inp] : view.each())
    {
        inp.update();
    };
}

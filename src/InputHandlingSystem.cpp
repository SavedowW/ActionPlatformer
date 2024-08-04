#include "InputHandlingSystem.h"
#include "CoreComponents.h"

InputHandlingSystem::InputHandlingSystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void InputHandlingSystem::update()
{
    auto view = m_reg.view<ComponentPlayerInput>();
    for (auto [idx, inp] : view.each())
    {
        inp.m_inputResolver->update();
    };
}

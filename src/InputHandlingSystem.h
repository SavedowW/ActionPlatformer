#pragma once
#include "Core/InputState.h"
#include "Core/InputSystem.h"
#include <entt/entt.hpp>

class InputHandlingSystem : public InputReactor
{
public:
    InputHandlingSystem(entt::registry &reg_);

    void receiveEvents(GAMEPLAY_EVENTS event_, float scale_) override;

    void update();
    void activate();
    void deactivate();

    void nullifyCurrentInput();

private:
    entt::registry &m_reg;
    InputState m_currentInput;
};


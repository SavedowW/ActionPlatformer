#include "InputHandlingSystem.h"
#include "Core/InputResolver.h"
#include "Core/Profile.h"
#include <stdexcept>

InputHandlingSystem::InputHandlingSystem(entt::registry &reg_) :
    m_reg(reg_)
{
    subscribe(GAMEPLAY_EVENTS::UP);
    subscribe(GAMEPLAY_EVENTS::RIGHT);
    subscribe(GAMEPLAY_EVENTS::DOWN);
    subscribe(GAMEPLAY_EVENTS::LEFT);
    subscribe(GAMEPLAY_EVENTS::ATTACK);

    activate();
}

void InputHandlingSystem::receiveEvents(GAMEPLAY_EVENTS event_, float scale_)
{
    switch (event_)
    {
        case (GAMEPLAY_EVENTS::UP):
            m_currentInput.m_inputs[INPUT_BUTTON::UP] = (scale_ > 0.0f ? (m_currentInput.m_inputs[INPUT_BUTTON::UP] != INPUT_BUTTON_STATE::HOLD ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::HOLD) : INPUT_BUTTON_STATE::RELEASED);
            break;

        case (GAMEPLAY_EVENTS::DOWN):
            m_currentInput.m_inputs[INPUT_BUTTON::DOWN] = (scale_ > 0.0f ? (m_currentInput.m_inputs[INPUT_BUTTON::DOWN] != INPUT_BUTTON_STATE::HOLD ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::HOLD) : INPUT_BUTTON_STATE::RELEASED);
            break;

        case (GAMEPLAY_EVENTS::LEFT):
            m_currentInput.m_inputs[INPUT_BUTTON::LEFT] = (scale_ > 0.0f ? (m_currentInput.m_inputs[INPUT_BUTTON::LEFT] != INPUT_BUTTON_STATE::HOLD ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::HOLD) : INPUT_BUTTON_STATE::RELEASED);
            break;

        case (GAMEPLAY_EVENTS::RIGHT):
            m_currentInput.m_inputs[INPUT_BUTTON::RIGHT] = (scale_ > 0.0f ? (m_currentInput.m_inputs[INPUT_BUTTON::RIGHT] != INPUT_BUTTON_STATE::HOLD ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::HOLD) : INPUT_BUTTON_STATE::RELEASED);
            break;
        
        case (GAMEPLAY_EVENTS::ATTACK):
            m_currentInput.m_inputs[INPUT_BUTTON::ATTACK] = (scale_ > 0.0f ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::RELEASED);
            break;

        default:
        break;
    }
}

void InputHandlingSystem::update()
{
    m_currentInput.setDirFromButtons();

    auto view = m_reg.view<components::InputResolver>();

    for (auto [idx, inputs] : view.each())
    {
        inputs.addFrame(m_currentInput);
    }

    m_currentInput = m_currentInput.getNextFrameState();
}

void InputHandlingSystem::activate()
{
    setInputEnabled();
}

void InputHandlingSystem::deactivate()
{
    setInputDisabled();
}

void InputHandlingSystem::nullifyCurrentInput()
{
    // TODO:
    throw std::runtime_error(std::string(__func__) + " not implemented");
}

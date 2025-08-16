#include "InputResolver.h"

void InputResolver::subscribePlayer()
{
    subscribe(GAMEPLAY_EVENTS::UP);
    subscribe(GAMEPLAY_EVENTS::RIGHT);
    subscribe(GAMEPLAY_EVENTS::DOWN);
    subscribe(GAMEPLAY_EVENTS::LEFT);
    subscribe(GAMEPLAY_EVENTS::ATTACK);
}

void InputResolver::unsubscribePlayer()
{
    unsubscribeFromAll();
}

void InputResolver::receiveEvents(GAMEPLAY_EVENTS event_, const float scale_)
{
    switch(event_)
    {
        case (GAMEPLAY_EVENTS::UP):
            m_currentInput.m_inputs[INPUT_BUTTON::UP] = (scale_ > 0.0f ? (m_currentInput.m_inputs[INPUT_BUTTON::UP]  != INPUT_BUTTON_STATE::HOLD ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::HOLD) : INPUT_BUTTON_STATE::RELEASED);
            break;

        case (GAMEPLAY_EVENTS::DOWN):
            m_currentInput.m_inputs[INPUT_BUTTON::DOWN] = (scale_ > 0.0f ? (m_currentInput.m_inputs[INPUT_BUTTON::DOWN]  != INPUT_BUTTON_STATE::HOLD ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::HOLD) : INPUT_BUTTON_STATE::RELEASED);
            break;

        case (GAMEPLAY_EVENTS::LEFT):
            m_currentInput.m_inputs[INPUT_BUTTON::LEFT] = (scale_ > 0.0f ? (m_currentInput.m_inputs[INPUT_BUTTON::LEFT]  != INPUT_BUTTON_STATE::HOLD ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::HOLD) : INPUT_BUTTON_STATE::RELEASED);
            break;

        case (GAMEPLAY_EVENTS::RIGHT):
            m_currentInput.m_inputs[INPUT_BUTTON::RIGHT] = (scale_ > 0.0f ? (m_currentInput.m_inputs[INPUT_BUTTON::RIGHT]  != INPUT_BUTTON_STATE::HOLD ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::HOLD) : INPUT_BUTTON_STATE::RELEASED);
            break;
        
        case (GAMEPLAY_EVENTS::ATTACK):
            m_currentInput.m_inputs[INPUT_BUTTON::ATTACK] = (scale_ > 0.0f ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::RELEASED);
            break;

        default:
    }
}

void InputResolver::update()
{
   //std::cout << "=== UPDATE RESOLVER ===\n";

    m_currentInput.setDirFromButtons();

    m_inputQueue.push(m_currentInput);

    m_currentInput = m_currentInput.getNextFrameState();
}

Vector2<int> InputResolver::getCurrentInputDir() const
{
    return m_currentInput.m_dir;
}

const InputQueue &InputResolver::getInputQueue() const
{
    return m_inputQueue;
}

INPUT_BUTTON_STATE InputResolver::getPostFrameButtonState(INPUT_BUTTON button_) const
{
    //return m_currentInput.inputs.at(button_);
    if (m_inputQueue.getFilled() >= 1)
    {
        return m_inputQueue[0].m_inputs.at(button_);
    }

    return INPUT_BUTTON_STATE::OFF;
}

void InputResolver::nullifyCurrentInput()
{
    for (const auto &btn : {INPUT_BUTTON::UP, INPUT_BUTTON::DOWN, INPUT_BUTTON::LEFT, INPUT_BUTTON::RIGHT, INPUT_BUTTON::ATTACK})
    {
        if (m_currentInput.m_inputs[btn] == INPUT_BUTTON_STATE::PRESSED || m_currentInput.m_inputs[btn] == INPUT_BUTTON_STATE::HOLD)
            m_currentInput.m_inputs[btn] = INPUT_BUTTON_STATE::RELEASED;
    }
}

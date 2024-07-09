#include "InputResolver.h"

InputResolver::InputResolver(InputSystem *input_) :
    InputReactor(input_)
{

}

void InputResolver::subscribePlayer()
{
    subscribe(EVENTS::UP);
    subscribe(EVENTS::RIGHT);
    subscribe(EVENTS::DOWN);
    subscribe(EVENTS::LEFT);
}

void InputResolver::unsubscribePlayer()
{
    unsubscribeFromAll();
}

void InputResolver::receiveInput(EVENTS event_, const float scale_)
{
    switch(event_)
    {
        case (EVENTS::UP):
            m_currentInput.m_inputs[INPUT_BUTTON::UP] = (scale_ == 1.0f ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::RELEASED);
            break;

        case (EVENTS::DOWN):
            m_currentInput.m_inputs[INPUT_BUTTON::DOWN] = (scale_ == 1.0f ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::RELEASED);
            break;

        case (EVENTS::LEFT):
            m_currentInput.m_inputs[INPUT_BUTTON::LEFT] = (scale_ == 1.0f ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::RELEASED);
            break;

        case (EVENTS::RIGHT):
            m_currentInput.m_inputs[INPUT_BUTTON::RIGHT] = (scale_ == 1.0f ? INPUT_BUTTON_STATE::PRESSED : INPUT_BUTTON_STATE::RELEASED);
            break;
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


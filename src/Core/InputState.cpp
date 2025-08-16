#include "InputState.h"

bool InputState::isInputActive(const INPUT_BUTTON &btn_) const
{
    auto inp = m_inputs.at(btn_);
    return (inp == INPUT_BUTTON_STATE::PRESSED || inp == INPUT_BUTTON_STATE::HOLD);
}

void InputState::setDirFromButtons()
{
    m_dir = Vector2{0.0f, 0.0f};
    m_dir.x += (isInputActive(INPUT_BUTTON::RIGHT) ? 1 : 0);
    m_dir.x -= (isInputActive(INPUT_BUTTON::LEFT) ? 1 : 0);

    m_dir.y += (isInputActive(INPUT_BUTTON::DOWN) ? 1 : 0);
    m_dir.y -= (isInputActive(INPUT_BUTTON::UP) ? 1 : 0);
}

InputState InputState::getNextFrameState() const
{
    InputState newState(*this);
    for (auto &el : newState.m_inputs)
    {
        if (el.second == INPUT_BUTTON_STATE::PRESSED)
            el.second = INPUT_BUTTON_STATE::HOLD;
        else if (el.second == INPUT_BUTTON_STATE::RELEASED)
            el.second = INPUT_BUTTON_STATE::OFF;
    }

    return newState;
}

InputState::InputState(const InputState &inputState_)
{
    m_dir = inputState_.m_dir;
    m_inputs = inputState_.m_inputs;
}

InputState &InputState::operator=(const InputState &rhs_)
{
    m_dir = rhs_.m_dir;
    m_inputs = rhs_.m_inputs;
    return *this;
}

InputState::InputState(InputState &&inputState_) noexcept
{
    m_dir = inputState_.m_dir;
    m_inputs = std::forward<std::map<INPUT_BUTTON, INPUT_BUTTON_STATE>>(inputState_.m_inputs);
}

InputState &InputState::operator=(InputState &&rhs_) noexcept
{
    m_dir = rhs_.m_dir;
    m_inputs = std::forward<std::map<INPUT_BUTTON, INPUT_BUTTON_STATE>>(rhs_.m_inputs);
    return *this;
}
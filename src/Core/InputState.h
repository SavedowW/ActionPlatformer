#pragma once
#include "FixedQueue.hpp"
#include "Vector2.hpp"
#include <map>

enum class INPUT_BUTTON : uint8_t {UP, DOWN, LEFT, RIGHT, ATTACK};
enum class INPUT_BUTTON_STATE : uint8_t {PRESSED, HOLD, RELEASED, OFF};

struct InputState
{
    std::map<INPUT_BUTTON, INPUT_BUTTON_STATE> m_inputs = {
        {INPUT_BUTTON::UP, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::DOWN, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::LEFT, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::RIGHT, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::ATTACK, INPUT_BUTTON_STATE::OFF}
    };
    Vector2<int> m_dir{0, 0};

    bool isInputActive(const INPUT_BUTTON &btn_) const;

    void setDirFromButtons();

    InputState getNextFrameState() const;

    InputState() = default;
    InputState(const InputState &inputState_);
    InputState &operator=(const InputState &rhs_);
    InputState(InputState &&inputState_) noexcept;
    InputState &operator=(InputState &&rhs_) noexcept;
};

std::ostream& operator<< (std::ostream& out_, const InputState& inState_);

using InputQueue = FixedQueue<InputState, 30>;

#ifndef INPUT_STATE_H_
#define INPUT_STATE_H_

#include "FixedQueue.hpp"
#include "Vector2.h"
#include <map>

enum class INPUT_BUTTON {UP, DOWN, LEFT, RIGHT, ATTACK};
enum class INPUT_BUTTON_STATE {PRESSED, HOLD, RELEASED, OFF};

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
    InputState(InputState &&inputState_);
    InputState &operator=(InputState &&rhs_);
};

inline std::ostream& operator<< (std::ostream& out_, const InputState& inState_)
{
    out_ << "(" << inState_.m_dir << "): ";
    for (auto &el : {INPUT_BUTTON::UP, INPUT_BUTTON::DOWN, INPUT_BUTTON::LEFT, INPUT_BUTTON::RIGHT, INPUT_BUTTON::ATTACK})
    {
        out_ << static_cast<int>(inState_.m_inputs.at(el));
    }
    return out_;
}

using InputQueue = FixedQueue<InputState, 30>;

#endif
#ifndef INPUT_STATE_H_
#define INPUT_STATE_H_

#include "Vector2.h"
#include <map>

enum class INPUT_BUTTON {A, B, C, S, UP, DOWN, LEFT, RIGHT};
enum class INPUT_BUTTON_STATE {PRESSED, HOLD, RELEASED, OFF};

struct InputState
{
    std::map<INPUT_BUTTON, INPUT_BUTTON_STATE> m_inputs = {
        {INPUT_BUTTON::A, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::B, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::C, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::S, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::UP, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::DOWN, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::LEFT, INPUT_BUTTON_STATE::OFF},
        {INPUT_BUTTON::RIGHT, INPUT_BUTTON_STATE::OFF}
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

inline std::ostream& operator<< (std::ostream& out_, InputState& inState_)
{
    out_ << "(" << inState_.m_dir << "): ";
    for (auto &el : {INPUT_BUTTON::A, INPUT_BUTTON::B, INPUT_BUTTON::C, INPUT_BUTTON::S, INPUT_BUTTON::UP, INPUT_BUTTON::DOWN, INPUT_BUTTON::LEFT, INPUT_BUTTON::RIGHT})
    {
        out_ << static_cast<int>(inState_.m_inputs[el]);
    }
    return out_;
}

#endif
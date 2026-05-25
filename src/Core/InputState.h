#pragma once
#include "FixedQueue.hpp"
#include "Vector2.hpp"
#include <map>

enum class INPUT_BUTTON : uint8_t {UP, DOWN, LEFT, RIGHT, ATTACK};
enum class INPUT_BUTTON_STATE : uint8_t {PRESSED, HOLD, RELEASED, OFF};

SERIALIZE_ENUM(INPUT_BUTTON, {
    ENUM_AUTO(INPUT_BUTTON, UP),
    ENUM_AUTO(INPUT_BUTTON, DOWN),
    ENUM_AUTO(INPUT_BUTTON, LEFT),
    ENUM_AUTO(INPUT_BUTTON, RIGHT),
    ENUM_AUTO(INPUT_BUTTON, ATTACK)
})

SERIALIZE_ENUM(INPUT_BUTTON_STATE, {
    ENUM_INIT(INPUT_BUTTON_STATE, PRESSED, "\\"),
    ENUM_INIT(INPUT_BUTTON_STATE, HOLD, "_"),
    ENUM_INIT(INPUT_BUTTON_STATE, RELEASED, "/"),
    ENUM_INIT(INPUT_BUTTON_STATE, OFF, "┬"),
})

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

using InputQueue = FixedQueue<InputState, 30>;

template <> 
struct std::formatter<InputState> : std::formatter<std::string_view> 
{
    auto format(const InputState &data_, format_context &ctx_) const 
    {
        return formatter<std::string_view>::format(
            std::format("{} {}:{}, {}:{}, {}:{}, {}:{}, {}:{}", data_.m_dir, 
                serialize(INPUT_BUTTON::UP), serialize(data_.m_inputs.at(INPUT_BUTTON::UP)),
                serialize(INPUT_BUTTON::DOWN), serialize(data_.m_inputs.at(INPUT_BUTTON::DOWN)),
                serialize(INPUT_BUTTON::LEFT), serialize(data_.m_inputs.at(INPUT_BUTTON::LEFT)),
                serialize(INPUT_BUTTON::RIGHT), serialize(data_.m_inputs.at(INPUT_BUTTON::RIGHT)),
                serialize(INPUT_BUTTON::ATTACK), serialize(data_.m_inputs.at(INPUT_BUTTON::UP))), 
            ctx_);
    }
};

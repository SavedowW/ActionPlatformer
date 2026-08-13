#pragma once
#include "InputState.h"
#include <unordered_map>

enum class InputMotions : uint8_t
{
    HOLD_HORDIR,
    CHECK_NO_HORDIR,
    HOLD_HORDIR_BUFFERED,

    TAP_ANY_EXCEPT_BACKWARDS,

    HOLD_UP,
    BUFFER_UP,
    BUFFER_UP_STRICT,
    
    HOLD_UP_FORWARD,
    BUFFER_UP_FORWARD,
    BUFFER_UP_FORWARD_STRICT,

    BUFFER_FORWARD_STRICT,

    BUFFER_DOWN_FORWARD_STRICT,

    BUFFER_DOWN_STRICT,

    BUFFERED_ORIENTED_ATTACK
};

SERIALIZE_ENUM(InputMotions, {
    ENUM_AUTO(InputMotions, HOLD_HORDIR),
    ENUM_AUTO(InputMotions, CHECK_NO_HORDIR),
    ENUM_AUTO(InputMotions, HOLD_HORDIR_BUFFERED),
    
    ENUM_AUTO(InputMotions, TAP_ANY_EXCEPT_BACKWARDS),

    ENUM_AUTO(InputMotions, HOLD_UP),
    ENUM_AUTO(InputMotions, BUFFER_UP),
    ENUM_AUTO(InputMotions, BUFFER_UP_STRICT),

    ENUM_AUTO(InputMotions, HOLD_UP_FORWARD),
    ENUM_AUTO(InputMotions, BUFFER_UP_FORWARD),
    ENUM_AUTO(InputMotions, BUFFER_UP_FORWARD_STRICT),

    ENUM_AUTO(InputMotions, BUFFER_FORWARD_STRICT),

    ENUM_AUTO(InputMotions, BUFFER_DOWN_FORWARD_STRICT),

    ENUM_AUTO(InputMotions, BUFFER_DOWN_STRICT),

    ENUM_AUTO(InputMotions, BUFFERED_ORIENTED_ATTACK)
})

class InputResolver
{
public:
    InputResolver();

    void addFrame(const InputState &currentInput_);

    Vector2<int> getCurrentInputDir() const;

    bool checkInput(InputMotions motion_, Orientation orientation_, unsigned int extendBuffer_) const;
    
    bool isInputActive(INPUT_BUTTON button_) const;

    const InputQueue &getHistory() const noexcept;

private:
    bool checkHoldHorDir(Orientation orientation_, unsigned int extendBuffer_) const;
    bool checkNoHorDir(Orientation orientation_, unsigned int extendBuffer_) const;
    bool checkBufferedHoldHorDir(Orientation orientation_, unsigned int extendBuffer_) const;

    bool checkTapExceptBackwards(Orientation orientation_, unsigned int extendBuffer_) const;

    bool checkHoldUp(Orientation orientation_, unsigned int extendBuffer_) const;
    bool checkBufferUp(Orientation orientation_, unsigned int extendBuffer_) const;
    bool checkBufferUpStrict(Orientation orientation_, unsigned int extendBuffer_) const;

    bool checkHoldUpForward(Orientation orientation_, unsigned int extendBuffer_) const;
    bool checkBufferUpForward(Orientation orientation_, unsigned int extendBuffer_) const;
    bool checkBufferUpForwardStrict(Orientation orientation_, unsigned int extendBuffer_) const;

    bool checkBufferedOrientedAttack(Orientation orientation_, unsigned int extendBuffer_) const;

    using InputCheck = bool (InputResolver::*)(Orientation, unsigned int) const;

    InputQueue m_inputQueue;
    std::unordered_map<InputMotions, InputCheck> m_inputComparators;
};

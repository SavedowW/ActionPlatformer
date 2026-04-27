#pragma once
#include "InputState.h"
#include <unordered_map>

enum class InputMotions : uint8_t
{
    HOLD_HORDIR,
    CHECK_NO_HORDIR,
    HOLD_UP,
    //HOLD_HORDIR_BUFFERED,
    //HOLD_DOWN,
    //TAP_UP,
    //TAP_DOWN,
    //TAP_UP_HORDIR,
    //TAP_ATTACK,
};

SERIALIZE_ENUM(InputMotions, {
    ENUM_AUTO(InputMotions, HOLD_HORDIR),
    ENUM_AUTO(InputMotions, CHECK_NO_HORDIR),
    ENUM_AUTO(InputMotions, HOLD_UP),
    //ENUM_AUTO(InputMotions, HOLD_HORDIR_BUFFERED),
    //ENUM_AUTO(InputMotions, HOLD_DOWN),
    //ENUM_AUTO(InputMotions, TAP_UP),
    //ENUM_AUTO(InputMotions, TAP_DOWN),
    //ENUM_AUTO(InputMotions, TAP_UP_HORDIR),
    //ENUM_AUTO(InputMotions, TAP_ATTACK)
})

class InputResolver
{
public:
    InputResolver();

    void addFrame(const InputState &currentInput_);

    Vector2<int> getCurrentInputDir() const;

    bool checkInput(InputMotions motion_, ORIENTATION orientation_, unsigned int extendBuffer_) const;
    
    bool isInputActive(INPUT_BUTTON button_) const;

private:
    bool checkHoldHorDir(ORIENTATION orientation_, unsigned int extendBuffer_) const;
    bool checkNoHorDir(ORIENTATION orientation_, unsigned int extendBuffer_) const;
    bool checkHoldUp(ORIENTATION orientation_, unsigned int extendBuffer_) const;
    //bool checkHoldHorDirBuffered(ORIENTATION orientation_, unsigned int extendBuffer_) const;
    //bool checkHoldDown(ORIENTATION orientation_, unsigned int extendBuffer_) const;
    //bool checkTapUp(ORIENTATION orientation_, unsigned int extendBuffer_) const;
    //bool checkTapDown(ORIENTATION orientation_, unsigned int extendBuffer_) const;
    //bool checkStrictTapUpHorDir(ORIENTATION orientation_, unsigned int extendBuffer_) const;
    //bool checkTapAttack(ORIENTATION orientation_, unsigned int extendBuffer_) const;

    using InputCheck = bool (InputResolver::*)(ORIENTATION, unsigned int) const;

    InputQueue m_inputQueue;
    std::unordered_map<InputMotions, InputCheck> m_inputComparators;
};

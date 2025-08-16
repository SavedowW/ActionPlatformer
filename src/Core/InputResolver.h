#ifndef INPUT_RESOLVER_H_
#define INPUT_RESOLVER_H_

#include "InputState.h"
#include "InputSystem.h"

class InputResolver : public InputReactor
{
public:
    void subscribePlayer();
    void unsubscribePlayer();
    void receiveEvents(GAMEPLAY_EVENTS event_, const float scale_) override;
    const InputQueue &getInputQueue() const;

    Vector2<int> getCurrentInputDir() const;
    INPUT_BUTTON_STATE getPostFrameButtonState(INPUT_BUTTON button_) const;

    void nullifyCurrentInput();

    void update();

protected:
    InputQueue m_inputQueue;
    InputState m_currentInput;
};

#endif

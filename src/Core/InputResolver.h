#ifndef INPUT_RESOLVER_H_
#define INPUT_RESOLVER_H_

#include "FixedQueue.hpp"
#include <map>
#include "InputState.h"
#include "InputSystem.h"
#include <vector>

class InputResolver : public InputReactor
{
public:
    InputResolver(InputSystem *input_);

    void subscribePlayer();
    void unsubscribePlayer();
    void receiveInput(EVENTS event_, const float scale_) override;
    const InputQueue &getInputQueue() const;

    Vector2<int> getCurrentInputDir() const;
    INPUT_BUTTON_STATE getPostFrameButtonState(INPUT_BUTTON button_) const;

    void update();

protected:
    InputQueue m_inputQueue;
    InputState m_currentInput;
};

#endif
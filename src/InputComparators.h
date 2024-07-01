#ifndef INPUT_COMPARATORS_H_
#define INPUT_COMPARATORS_H_

#include "FixedQueue.h"
#include "Vector2.h"
#include "InputState.h"

using InputQueue = FixedQueue<InputState, 30>;

class InputComparator
{
public:
    virtual bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_ = 0) const = 0;
};

class InputComparatorIdle : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorForward : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorBackward : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorUpPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorUpHold : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorUpForwardPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorUpForwardHold : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorUpBackwardPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorUpBackwardHold : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorDownHold : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparator66 : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparator44 : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorAPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorBPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparator2BPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparator4APress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorCPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorSPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorSHold : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparator2CPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparator214CPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparator236CPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparatorBCPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparator4BCPress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};

class InputComparator214APress : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const final;
};


using InputComparator_ptr = std::unique_ptr<InputComparator>;

#endif
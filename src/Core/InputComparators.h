#ifndef INPUT_COMPARATORS_H_
#define INPUT_COMPARATORS_H_

#include "Vector2.h"
#include "InputState.h"

class InputComparator
{
public:
    virtual bool operator()(const InputQueue &inputQueue_, int extendBuffer_ = 0) const = 0;
};

class InputComparatorIdle : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, int extendBuffer_) const final;
};

class InputComparatorHoldRight : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, int extendBuffer_) const final;
};

class InputComparatorHoldLeft : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, int extendBuffer_) const final;
};

using InputComparator_ptr = std::unique_ptr<InputComparator>;

#endif
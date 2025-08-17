#ifndef INPUT_COMPARATORS_H_
#define INPUT_COMPARATORS_H_

#include "InputState.h"

class InputComparatorIdle
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorHoldRight
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorHoldLeft
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorBufferedHoldRight
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorBufferedHoldLeft
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorHoldUp
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorHoldDown
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};


class InputComparatorTapUp
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorTapAnyDown
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorTapUpRight
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorTapUpLeft
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorFail
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorTapAttack
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorTapAnyRight
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

class InputComparatorTapAnyLeft
{
public:
    static bool check(const InputQueue &inputQueue_, unsigned int extendBuffer_);
};

#endif
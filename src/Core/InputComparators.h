#ifndef INPUT_COMPARATORS_H_
#define INPUT_COMPARATORS_H_

#include "InputState.h"

class InputComparator
{
public:
    virtual bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_ = 0) const = 0;
};

class InputComparatorIdle : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorHoldRight : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorHoldLeft : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorBufferedHoldRight : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorBufferedHoldLeft : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorHoldUp : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorHoldDown : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};


class InputComparatorTapUp : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorTapAnyDown : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorTapUpRight : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorTapUpLeft : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorFail : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorTapAttack : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorTapAnyRight : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

class InputComparatorTapAnyLeft : public InputComparator
{
public:
    bool operator()(const InputQueue &inputQueue_, unsigned int extendBuffer_) const final;
};

#endif
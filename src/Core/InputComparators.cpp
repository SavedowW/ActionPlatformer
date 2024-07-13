#include "InputComparators.h"
#include "GameData.h"

template<size_t len, typename VecT>
bool recursivelySearchInput(const InputQueue &inputQueue_, Vector2<VecT> (&inputs_)[len], int start_, int toSearch_, int window_)
{
    int lastToLook = std::min(inputQueue_.getFilled() - 1, start_ + window_ - 1);
    for (int i = start_; i < lastToLook; ++i)
    {
        auto &inp = inputQueue_[i];
        if (inp.m_dir == inputs_[toSearch_])
        {
            if (toSearch_ == len - 1)
                return true;
            else if (recursivelySearchInput(inputQueue_, inputs_, i + 1, toSearch_ + 1, window_))
                return true;
        }
    }
    return false;
}

bool InputComparatorIdle::operator()(const InputQueue &inputQueue_, int extendBuffer_) const
{
    return true;
}

bool InputComparatorHoldRight::operator()(const InputQueue &inputQueue_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    return inputQueue_[0].m_dir.x > 0;
}

bool InputComparatorHoldLeft::operator()(const InputQueue &inputQueue_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    return inputQueue_[0].m_dir.x < 0;
}

bool InputComparatorTapUp::operator()(const InputQueue &inputQueue_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        bool valid = in.m_dir.y == -1.0f && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED;
        if (valid)
            return true;
    }

    return false;
}

bool InputComparatorTapAnyDown::operator()(const InputQueue &inputQueue_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        bool valid = in.m_dir.y == 1.0f && in.m_inputs.at(INPUT_BUTTON::DOWN) == INPUT_BUTTON_STATE::PRESSED;
        if (valid)
            return true;
    }

    return false;
}


bool InputComparatorTapUpRight::operator()(const InputQueue &inputQueue_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_dir == Vector2{1.0f, -1.0f} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED)
            return true;
    }

    return false;
}

bool InputComparatorTapUpLeft::operator()(const InputQueue &inputQueue_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_dir == Vector2{-1.0f, -1.0f} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED)
            return true;
    }

    return false;
}

bool InputComparatorHoldUp::operator()(const InputQueue &inputQueue_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    return inputQueue_[0].m_dir.y < 0;
}

bool InputComparatorFail::operator()(const InputQueue &inputQueue_, int extendBuffer_) const
{
    return false;
}

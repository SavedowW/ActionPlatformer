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

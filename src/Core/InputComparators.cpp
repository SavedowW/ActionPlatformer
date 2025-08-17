#include "InputComparators.h"
#include "GameData.h"

#if 0 // was used for FG-ish motion inputs
template<size_t len, typename VecT>
bool recursivelySearchInput(const InputQueue &inputQueue_, Vector2<VecT> (&inputs_)[len], int start_, int toSearch_, int window_)
{
    assert(inputQueue_.getFilled());

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
#endif

bool InputComparatorIdle::check(const InputQueue&, unsigned int)
{
    return true;
}

bool InputComparatorHoldRight::check(const InputQueue &inputQueue_, unsigned int)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    return inputQueue_[0].m_dir.x > 0;
}

bool InputComparatorHoldLeft::check(const InputQueue &inputQueue_, unsigned int)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    return inputQueue_[0].m_dir.x < 0;
}

bool InputComparatorTapUp::check(const InputQueue &inputQueue_, unsigned int extendBuffer_)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    size_t lookAt = std::min(inputQueue_.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        bool valid = in.m_dir.y == -1.0f && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED;
        if (valid)
            return true;
    }

    return false;
}

bool InputComparatorTapAnyDown::check(const InputQueue &inputQueue_, unsigned int extendBuffer_)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    size_t lookAt = std::min(inputQueue_.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        bool valid = in.m_dir.y == 1.0f && in.m_inputs.at(INPUT_BUTTON::DOWN) == INPUT_BUTTON_STATE::PRESSED;
        if (valid)
            return true;
    }

    return false;
}


bool InputComparatorTapUpRight::check(const InputQueue &inputQueue_, unsigned int extendBuffer_)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    size_t lookAt = std::min(inputQueue_.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_dir == Vector2{1.0f, -1.0f} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED)
            return true;
    }

    return false;
}

bool InputComparatorTapUpLeft::check(const InputQueue &inputQueue_, unsigned int extendBuffer_)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    size_t lookAt = std::min(inputQueue_.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_dir == Vector2{-1.0f, -1.0f} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED)
            return true;
    }

    return false;
}

bool InputComparatorHoldUp::check(const InputQueue &inputQueue_, unsigned int)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    return inputQueue_[0].m_dir.y < 0;
}

bool InputComparatorHoldDown::check(const InputQueue &inputQueue_, unsigned int)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    return inputQueue_[0].m_dir.y > 0;
}

bool InputComparatorFail::check(const InputQueue&, unsigned int)
{
    return false;
}

bool InputComparatorTapAttack::check(const InputQueue &inputQueue_, unsigned int extendBuffer_)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    size_t lookAt = std::min(inputQueue_.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::ATTACK) == INPUT_BUTTON_STATE::PRESSED)
            return true;
    }

    return false;
}

bool InputComparatorTapAnyRight::check(const InputQueue &inputQueue_, unsigned int extendBuffer_)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    size_t lookAt = std::min(inputQueue_.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_dir.x != -1 && in.m_dir != Vector2{0.0f, 0.0f} && (
            in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED ||
            in.m_inputs.at(INPUT_BUTTON::RIGHT) == INPUT_BUTTON_STATE::PRESSED ||
            in.m_inputs.at(INPUT_BUTTON::DOWN) == INPUT_BUTTON_STATE::PRESSED
        ))
            return true;
    }

    return false;
}

bool InputComparatorTapAnyLeft::check(const InputQueue &inputQueue_, unsigned int extendBuffer_)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    size_t lookAt = std::min(inputQueue_.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_dir.x != 1 && in.m_dir != Vector2{0.0f, 0.0f} && (
            in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED ||
            in.m_inputs.at(INPUT_BUTTON::LEFT) == INPUT_BUTTON_STATE::PRESSED ||
            in.m_inputs.at(INPUT_BUTTON::DOWN) == INPUT_BUTTON_STATE::PRESSED
        ))
            return true;
    }

    return false;
}

bool InputComparatorBufferedHoldRight::check(const InputQueue &inputQueue_, unsigned int extendBuffer_)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    size_t lookAt = std::min(inputQueue_.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_dir.x > 0)
            return true;
    }

    return false;
}

bool InputComparatorBufferedHoldLeft::check(const InputQueue &inputQueue_, unsigned int extendBuffer_)
{
    if (inputQueue_.getFilled() == 0)
        return false;

    size_t lookAt = std::min(inputQueue_.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_dir.x < 0)
            return true;
    }

    return false;
}

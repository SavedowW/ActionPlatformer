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

bool InputComparatorIdle::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    return true;
}

bool InputComparatorForward::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    switch (faceDirection_)
    {
        case (ORIENTATION::RIGHT):
            return (inputQueue_[0].m_dir == Vector2{1.0f, 0.0f});
            break;

        case (ORIENTATION::LEFT):
            return (inputQueue_[0].m_dir == Vector2{-1.0f, 0.0f});
            break;
    }

    return false;
}

bool InputComparatorBackward::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    switch (faceDirection_)
    {
        case (ORIENTATION::RIGHT):
            return (inputQueue_[0].m_dir == Vector2{-1.0f, 0.0f});
            break;

        case (ORIENTATION::LEFT):
            return (inputQueue_[0].m_dir == Vector2{1.0f, 0.0f});
            break;
    }

    return false;
}

bool InputComparatorUpPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        bool valid = in.m_dir == Vector2{0.0f, -1.0f} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED;
        if (valid)
            return true;
    }

    return false;

    //return (lastInput.dir == Vector2{0.0f, -1.0f} && lastInput.inputs[INPUT_BUTTON::UP] == INPUT_BUTTON_STATE::PRESSED);
}

bool InputComparatorUpHold::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    return (inputQueue_[0].m_dir == Vector2{0.0f, -1.0f});
}

bool InputComparatorUpForwardPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        bool valid = false;

        switch (faceDirection_)
        {
            case (ORIENTATION::RIGHT):
                valid = (in.m_dir == Vector2{1.0f, -1.0f} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED);
                break;

            case (ORIENTATION::LEFT):
                valid = (in.m_dir == Vector2{-1.0f, -1.0f} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED);
                break;
        }

        if (valid)
            return true;
    }

    return false;
}

bool InputComparatorUpForwardHold::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    auto lastInput = inputQueue_[0];

    switch (faceDirection_)
    {
        case (ORIENTATION::RIGHT):
            return (lastInput.m_dir == Vector2{1.0f, -1.0f});
            break;

        case (ORIENTATION::LEFT):
            return (lastInput.m_dir == Vector2{-1.0f, -1.0f});
            break;
    }

    return false;
}

bool InputComparatorUpBackwardPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        bool valid = false;

        switch (faceDirection_)
        {
            case (ORIENTATION::RIGHT):
                valid = (in.m_dir == Vector2{-1.0f, -1.0f} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED);
                break;

            case (ORIENTATION::LEFT):
                valid = (in.m_dir == Vector2{1.0f, -1.0f} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED);
                break;
        }

        if (valid)
            return true;
    }

    return false;
}

bool InputComparatorUpBackwardHold::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    auto lastInput = inputQueue_[0];

    switch (faceDirection_)
    {
        case (ORIENTATION::RIGHT):
            return (lastInput.m_dir == Vector2{-1.0f, -1.0f});
            break;

        case (ORIENTATION::LEFT):
            return (lastInput.m_dir == Vector2{1.0f, -1.0f});
            break;
    }

    return false;
}

bool InputComparatorDownHold::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    auto lastInput = inputQueue_[0];

    return (lastInput.m_dir.y == 1);
}

bool InputComparator66::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() <= 1)
        return false;

    Vector2<float> forwardVec;
    forwardVec.x = (faceDirection_ == ORIENTATION::RIGHT ? 1.0f : -1.0f);
    Vector2<float> upForwardVec = {forwardVec.x, -1.0f};
    INPUT_BUTTON forwardButton = (faceDirection_ == ORIENTATION::RIGHT ? INPUT_BUTTON::RIGHT : INPUT_BUTTON::LEFT);

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        const auto &in = inputQueue_[i];

        if (in.m_dir == forwardVec && in.m_inputs.at(forwardButton) == INPUT_BUTTON_STATE::PRESSED)
        {
            int lastToParse = std::min(i + 10, inputQueue_.getFilled() - 1);

            for (int k = i + 1; k <= lastToParse; ++k)
            {
                const auto &in2 = inputQueue_[k];
                if ((in2.m_dir == forwardVec || in2.m_dir == upForwardVec) && in2.m_inputs.at(forwardButton) == INPUT_BUTTON_STATE::PRESSED)
                    return true;
            }
        }
    }

    return false;
}

bool InputComparator44::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() <= 1)
        return false;

    Vector2<float> backwardVec;
    backwardVec.x = (faceDirection_ == ORIENTATION::RIGHT ? -1.0f : 1.0f);
    Vector2<float> upBackwardVec = {backwardVec.x, -1.0f};
    INPUT_BUTTON backwardButton = (faceDirection_ == ORIENTATION::RIGHT ? INPUT_BUTTON::LEFT : INPUT_BUTTON::RIGHT);

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        const auto &in = inputQueue_[i];

        if (in.m_dir == backwardVec && in.m_inputs.at(backwardButton) == INPUT_BUTTON_STATE::PRESSED)
        {
            int lastToParse = std::min(i + 10, inputQueue_.getFilled() - 1);

            for (int k = i + 1; k <= lastToParse; ++k)
            {
                const auto &in2 = inputQueue_[k];
                if ((in2.m_dir == backwardVec || in2.m_dir == upBackwardVec) && in2.m_inputs.at(backwardButton) == INPUT_BUTTON_STATE::PRESSED)
                    return true;
            }
        }
    }

    return false;
}

bool InputComparatorAPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::A) == INPUT_BUTTON_STATE::PRESSED)
        {
            return true;
        }
    }

    return false;
}

bool InputComparatorBPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::B) == INPUT_BUTTON_STATE::PRESSED)
        {
            return true;
        }
    }

    return false;
}

bool InputComparatorCPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::C) == INPUT_BUTTON_STATE::PRESSED)
        {
            return true;
        }
    }

    return false;
}

bool InputComparator2BPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::B) == INPUT_BUTTON_STATE::PRESSED && in.m_dir.y == 1)
        {
            return true;
        }
    }

    return false;
}

bool InputComparator4APress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    float backdir = (faceDirection_ == ORIENTATION::RIGHT ? -1.0f : 1.0f);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::A) == INPUT_BUTTON_STATE::PRESSED && in.m_dir.x == backdir)
        {
            return true;
        }
    }

    return false;
}

bool InputComparator2CPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::C) == INPUT_BUTTON_STATE::PRESSED && in.m_dir.y == 1)
        {
            return true;
        }
    }

    return false;
}

bool InputComparator214CPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    int secondaryInputButtonWindow = 5;
    int back = (faceDirection_ == ORIENTATION::RIGHT ? -1 : 1);
    Vector2<int> inputs[] = {{back, 0}, {back, 1}, {0, 1}};
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::C) == INPUT_BUTTON_STATE::PRESSED)
        {
            return recursivelySearchInput(inputQueue_, inputs, i, 0, secondaryInputButtonWindow);
        }
    }

    return false;
}

bool InputComparator236CPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    int secondaryInputButtonWindow = 5;
    int forward = (faceDirection_ == ORIENTATION::RIGHT ? 1 : -1);
    Vector2<int> inputs[] = {{forward, 0}, {forward, 1}, {0, 1}};
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::C) == INPUT_BUTTON_STATE::PRESSED)
        {
            return recursivelySearchInput(inputQueue_, inputs, i, 0, secondaryInputButtonWindow);
        }
    }

    return false;
}

bool InputComparatorBCPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    std::array<INPUT_BUTTON, 2> buttons{INPUT_BUTTON::B, INPUT_BUTTON::C};
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        bool allPressed = true;
        for (const auto &btn : buttons)
            if (!in.isInputActive(btn))
                allPressed = false;
        
        if (allPressed)
        {
            for (const auto &btn : buttons)
                if (in.m_inputs.at(btn) == INPUT_BUTTON_STATE::PRESSED)
                    return true;
        }
    }

    return false;
}

bool InputComparator4BCPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    std::array<INPUT_BUTTON, 2> buttons{INPUT_BUTTON::B, INPUT_BUTTON::C};
    float backDir = (faceDirection_ == ORIENTATION::RIGHT ? -1.0f : 1.0f);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        bool allPressed = true;

        if (in.m_dir.x == backDir)
        {
            for (const auto &btn : buttons)
                if (!in.isInputActive(btn))
                    allPressed = false;

            if (allPressed)
            {
                for (const auto &btn : buttons)
                    if (in.m_inputs.at(btn) == INPUT_BUTTON_STATE::PRESSED)
                        return true;
            }
        }
    }

    return false;
}

bool InputComparator214APress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    int secondaryInputButtonWindow = 5;
    int back = (faceDirection_ == ORIENTATION::RIGHT ? -1 : 1);
    Vector2<int> inputs[] = {{back, 0}, {back, 1}, {0, 1}};
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::A) == INPUT_BUTTON_STATE::PRESSED)
        {
            return recursivelySearchInput(inputQueue_, inputs, i, 0, secondaryInputButtonWindow);
        }
    }

    return false;
}

bool InputComparatorSPress::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    int lookAt = std::min(inputQueue_.getFilled() - 1, gamedata::global::inputBufferLength + extendBuffer_);
    for (int i = 0; i <= lookAt; ++i)
    {
        auto &in = inputQueue_[i];
        if (in.m_inputs.at(INPUT_BUTTON::S) == INPUT_BUTTON_STATE::PRESSED)
        {
            return true;
        }
    }

    return false;
}

bool InputComparatorSHold::operator()(const InputQueue &inputQueue_, ORIENTATION faceDirection_, int extendBuffer_) const
{
    if (inputQueue_.getFilled() == 0)
        return false;

    return inputQueue_[0].isInputActive(INPUT_BUTTON::S);
}

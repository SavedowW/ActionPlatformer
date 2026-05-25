#include "InputResolver.h"
#include "GameData.h"
#include "InputState.h"
#include "Vector2.hpp"

InputResolver::InputResolver() :
    m_inputComparators {
        {InputMotions::HOLD_HORDIR, &InputResolver::checkHoldHorDir},
        {InputMotions::CHECK_NO_HORDIR, &InputResolver::checkNoHorDir},
        {InputMotions::HOLD_HORDIR_BUFFERED, &InputResolver::checkBufferedHoldHorDir},

        {InputMotions::TAP_ANY_EXCEPT_BACKWARDS, &InputResolver::checkTapExceptBackwards},

        {InputMotions::HOLD_UP, &InputResolver::checkHoldUp},
        {InputMotions::BUFFER_UP, &InputResolver::checkBufferUp},
        {InputMotions::BUFFER_UP_STRICT, &InputResolver::checkBufferUpStrict},

        {InputMotions::HOLD_UP_FORWARD, &InputResolver::checkHoldUpForward},
        {InputMotions::BUFFER_UP_FORWARD, &InputResolver::checkBufferUpForward},
        {InputMotions::BUFFER_UP_FORWARD_STRICT, &InputResolver::checkBufferUpForwardStrict},
        //{InputMotions::HOLD_DOWN, &InputResolver::checkHoldDown},
        //{InputMotions::TAP_UP, &InputResolver::checkTapUp},
        //{InputMotions::TAP_DOWN, &InputResolver::checkTapDown},
        //{InputMotions::TAP_UP_HORDIR, &InputResolver::checkStrictTapUpHorDir},
        //{InputMotions::TAP_ATTACK, &InputResolver::checkTapAttack}
    }
{}

void InputResolver::addFrame(const InputState &currentInput_)
{
    m_inputQueue.push(currentInput_);
}


bool InputResolver::checkHoldHorDir(const ORIENTATION orientation_, unsigned int) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    const int expected = (orientation_ == ORIENTATION::RIGHT ? 1 : -1);

    return m_inputQueue[0].m_dir.x == expected;
}

bool InputResolver::checkNoHorDir(const ORIENTATION orientation_, unsigned int) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    const int expected = (orientation_ == ORIENTATION::RIGHT ? 1 : -1);

    return m_inputQueue[0].m_dir.x != expected;
}

bool InputResolver::checkHoldUp(ORIENTATION, unsigned int) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    return m_inputQueue[0].m_dir.y < 0;
}

bool InputResolver::checkHoldUpForward(const ORIENTATION orientation_, unsigned int) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    if (m_inputQueue[0].m_dir.y >= 0)
        return false;

    if (orientation_ == ORIENTATION::RIGHT)
        return m_inputQueue[0].m_dir.x > 0;

    return m_inputQueue[0].m_dir.x < 0;
}

bool InputResolver::checkBufferUp(ORIENTATION, const unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        if (in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED && in.m_dir.y < 0)
            return true;
    }

    return false;
}

bool InputResolver::checkBufferUpStrict(ORIENTATION, const unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    const size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        if (in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED && in.m_dir == Vector2{0, -1})
            return true;
    }

    return false;
}

bool InputResolver::checkBufferUpForward(const ORIENTATION orientation_, const unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    const auto expectedButton = (orientation_ == ORIENTATION::RIGHT ? INPUT_BUTTON::RIGHT : INPUT_BUTTON::LEFT);
    const auto expectedDir = (orientation_ == ORIENTATION::RIGHT ? 1 : -1);

    const size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        if (in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED && in.m_dir.y < 0 &&
            in.m_inputs.at(expectedButton) == INPUT_BUTTON_STATE::HOLD && in.m_dir.x == expectedDir)
            return true;
    }

    return false;
}

bool InputResolver::checkBufferUpForwardStrict(const ORIENTATION orientation_, const unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    const auto expectedButton = (orientation_ == ORIENTATION::RIGHT ? INPUT_BUTTON::RIGHT : INPUT_BUTTON::LEFT);
    const auto expectedDir = (orientation_ == ORIENTATION::RIGHT ? 1 : -1);

    bool foundUp = false;
    bool foundForward = false;
    bool foundDirection = false;

    const size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        foundUp = foundUp || in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED;
        foundForward = foundForward || in.m_inputs.at(expectedButton) == INPUT_BUTTON_STATE::PRESSED;
        foundDirection = foundDirection || (in.m_dir == Vector2{expectedDir, -1});

        if ((foundUp || foundForward) && foundDirection)
            return true;
    }

    return false;
}

bool InputResolver::checkBufferedHoldHorDir(const ORIENTATION orientation_, const unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    const int expected = (orientation_ == ORIENTATION::RIGHT ? 1 : -1);

    const size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        if (in.m_dir.x == expected)
            return true;
    }

    return false;
}

bool InputResolver::checkTapExceptBackwards(const ORIENTATION orientation_, const unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    const auto expectedButton = (orientation_ == ORIENTATION::RIGHT ? INPUT_BUTTON::RIGHT : INPUT_BUTTON::LEFT);
    const int expected = (orientation_ == ORIENTATION::RIGHT ? 1 : -1);

    bool vUpDirDiscovered = false;
    bool vDownDirDiscovered = false;
    bool horDirDiscovered = false;

    const size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        vUpDirDiscovered = vUpDirDiscovered || in.m_dir.y < 0;
        vDownDirDiscovered = vDownDirDiscovered || in.m_dir.y > 0;
        horDirDiscovered = horDirDiscovered || in.m_dir.x == expected;

        if (in.m_inputs.at(expectedButton) == INPUT_BUTTON_STATE::PRESSED && horDirDiscovered ||
            in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED && vUpDirDiscovered ||
            in.m_inputs.at(INPUT_BUTTON::DOWN) == INPUT_BUTTON_STATE::PRESSED && vDownDirDiscovered)
            return true;
    }

    return false;
}


Vector2<int> InputResolver::getCurrentInputDir() const
{
    if (m_inputQueue.getFilled() > 0)
        return m_inputQueue[0].m_dir;

    return {0, 0};
}

bool InputResolver::checkInput(InputMotions motion_, ORIENTATION orientation_, unsigned int extendBuffer_) const
{
    return (this->*m_inputComparators.at(motion_))(orientation_, extendBuffer_);
}

bool InputResolver::isInputActive(const INPUT_BUTTON button_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    return m_inputQueue[0].isInputActive(button_);
}

const InputQueue &InputResolver::getHistory() const noexcept
{
    return m_inputQueue;
}

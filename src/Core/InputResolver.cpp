#include "InputResolver.h"
#include "GameData.h"
#include "InputState.h"
#include "Vector2.hpp"

InputResolver::InputResolver() :
    m_inputComparators {
        {InputMotions::HOLD_HORDIR, &InputResolver::checkHoldHorDir},
        {InputMotions::CHECK_NO_HORDIR, &InputResolver::checkNoHorDir},
        {InputMotions::HOLD_UP, &InputResolver::checkHoldUp},
        {InputMotions::HOLD_UP_FORWARD, &InputResolver::checkHoldUpForward},
        {InputMotions::BUFFER_UP, &InputResolver::checkBufferUp},
        {InputMotions::BUFFER_UP_FORWARD, &InputResolver::checkBufferUpForward},
        {InputMotions::HOLD_HORDIR_BUFFERED, &InputResolver::checkBufferedHoldHorDir},
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
            in.m_inputs.at(expectedButton) == INPUT_BUTTON_STATE::PRESSED && in.m_dir.x == expectedDir)
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


/*
bool InputResolver::checkHoldDown(ORIENTATION, unsigned int) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    return m_inputQueue[0].m_dir.y > 0;
}

bool InputResolver::checkTapUp(ORIENTATION, unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        if (in.m_dir.y == -1 && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED)
            return true;
    }

    return false;
}

bool InputResolver::checkTapDown(ORIENTATION, unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        if (in.m_dir.y == 1 && in.m_inputs.at(INPUT_BUTTON::DOWN) == INPUT_BUTTON_STATE::PRESSED)
            return true;
    }

    return false;
}

bool InputResolver::checkStrictTapUpHorDir(ORIENTATION orientation_, unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    const int expected = (orientation_ == ORIENTATION::RIGHT ? 1 : -1);

    size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        if (in.m_dir == Vector2{expected, -1} && in.m_inputs.at(INPUT_BUTTON::UP) == INPUT_BUTTON_STATE::PRESSED)
            return true;
    }

    return false;
}

bool InputResolver::checkTapAttack(ORIENTATION, unsigned int extendBuffer_) const
{
    if (m_inputQueue.getFilled() == 0)
        return false;

    size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
    for (size_t i = 0; i <= lookAt; ++i)
    {
        const auto &in = m_inputQueue[i];
        if (in.m_inputs.at(INPUT_BUTTON::ATTACK) == INPUT_BUTTON_STATE::PRESSED)
            return true;
    }

    return false;
}*/


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

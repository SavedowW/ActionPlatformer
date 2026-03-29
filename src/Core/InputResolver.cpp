#include "InputResolver.h"
#include "GameData.h"
#include "Vector2.hpp"

namespace components
{
    InputResolver::InputResolver() :
        m_inputComparators {
            {InputMotions::FAIL, &InputResolver::checkAlwaysFail},
            {InputMotions::SUCCESS, &InputResolver::checkAlwaysSuccess},
            {InputMotions::HOLD_HORDIR, &InputResolver::checkHoldHorDir},
            {InputMotions::HOLD_HORDIR_BUFFERED, &InputResolver::checkHoldHorDirBuffered},
            {InputMotions::HOLD_UP, &InputResolver::checkHoldUp},
            {InputMotions::HOLD_DOWN, &InputResolver::checkHoldDown},
            {InputMotions::TAP_UP, &InputResolver::checkTapUp},
            {InputMotions::TAP_DOWN, &InputResolver::checkTapDown},
            {InputMotions::TAP_UP_HORDIR, &InputResolver::checkStrictTapUpHorDir},
            {InputMotions::TAP_ATTACK, &InputResolver::checkTapAttack}
        }
    {}

    void InputResolver::addFrame(const InputState &currentInput_)
    {
        m_inputQueue.push(currentInput_);
    }


    bool InputResolver::checkAlwaysFail(ORIENTATION, unsigned int) const
    {
        return false;
    }

    bool InputResolver::checkAlwaysSuccess(ORIENTATION, unsigned int) const
    {
        return true;
    }

    bool InputResolver::checkHoldHorDir(ORIENTATION orientation_, unsigned int) const
    {
        if (m_inputQueue.getFilled() == 0)
            return false;

        const int expected = (orientation_ == ORIENTATION::RIGHT ? 1 : -1);

        return m_inputQueue[0].m_dir.x == expected;
    }

    bool InputResolver::checkHoldHorDirBuffered(ORIENTATION orientation_, unsigned int extendBuffer_) const
    {
        if (m_inputQueue.getFilled() == 0)
            return false;

        const int expected = (orientation_ == ORIENTATION::RIGHT ? 1 : -1);

        size_t lookAt = std::min(m_inputQueue.getFilled() - 1, static_cast<size_t>(gamedata::global::inputBufferLength + extendBuffer_));
        for (size_t i = 0; i <= lookAt; ++i)
        {
            const auto &in = m_inputQueue[i];
            if (in.m_dir.x == expected)
                return true;
        }

        return false;
    }

    bool InputResolver::checkHoldUp(ORIENTATION, unsigned int) const
    {
        if (m_inputQueue.getFilled() == 0)
            return false;

        return m_inputQueue[0].m_dir.y < 0;
    }

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
}

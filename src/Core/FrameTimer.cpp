#include "FrameTimer.h"

FrameTimer::FrameTimer(uint32_t framesToWait_)
{
    begin(framesToWait_);
}

void FrameTimer::begin(uint32_t framesToWait_)
{
    m_currentFrame = 0;
    m_framesToWait = framesToWait_;
}

bool FrameTimer::update()
{
    if (m_framesToWait == 0)
        return false;

    if (m_currentFrame < m_framesToWait)
        m_currentFrame++;

    return m_currentFrame >= m_framesToWait;
}

bool FrameTimer::isOver() const
{
    if (m_framesToWait == 0)
        return false;

    return m_currentFrame >= m_framesToWait;
}

bool FrameTimer::isActive() const
{
    if (m_framesToWait == 0)
        return false;

    return m_currentFrame < m_framesToWait;
}

uint32_t FrameTimer::getCurrentFrame() const
{
    return m_currentFrame;
}

float FrameTimer::getProgressNormalized() const
{
    if (m_framesToWait == 0)
        return 0;

    return (float)m_currentFrame / m_framesToWait;
}
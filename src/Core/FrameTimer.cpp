#include "FrameTimer.h"

template<bool RESULT_IF_INACTIVE>
FrameTimer<RESULT_IF_INACTIVE>::FrameTimer(uint32_t framesToWait_)
{
    begin(framesToWait_);
}

template<bool RESULT_IF_INACTIVE>
void FrameTimer<RESULT_IF_INACTIVE>::begin(uint32_t framesToWait_)
{
    m_currentFrame = 0;
    m_framesToWait = framesToWait_;
}

template <bool RESULT_IF_INACTIVE>
void FrameTimer<RESULT_IF_INACTIVE>::beginAt(uint32_t framesToWait_, float progress_)
{
    m_currentFrame = framesToWait_ * progress_;
    m_framesToWait = framesToWait_;
}

template <bool RESULT_IF_INACTIVE>
void FrameTimer<RESULT_IF_INACTIVE>::setCurrentFrame(uint32_t frame_)
{
    m_currentFrame = frame_;
}

template <bool RESULT_IF_INACTIVE>
uint32_t FrameTimer<RESULT_IF_INACTIVE>::getDuration() const
{
    return m_framesToWait;
}

template<bool RESULT_IF_INACTIVE>
bool FrameTimer<RESULT_IF_INACTIVE>::update()
{
    if (m_framesToWait == 0)
        return RESULT_IF_INACTIVE;

    if (m_currentFrame < m_framesToWait)
        m_currentFrame++;

    return m_currentFrame >= m_framesToWait;
}

template<bool RESULT_IF_INACTIVE>
bool FrameTimer<RESULT_IF_INACTIVE>::isOver() const
{
    if (m_framesToWait == 0)
        return RESULT_IF_INACTIVE;

    return m_currentFrame >= m_framesToWait;
}

template<bool RESULT_IF_INACTIVE>
bool FrameTimer<RESULT_IF_INACTIVE>::isActive() const
{
    if (m_framesToWait == 0)
        return false;

    return m_currentFrame < m_framesToWait;
}

template<bool RESULT_IF_INACTIVE>
uint32_t FrameTimer<RESULT_IF_INACTIVE>::getCurrentFrame() const
{
    return m_currentFrame;
}

template<bool RESULT_IF_INACTIVE>
float FrameTimer<RESULT_IF_INACTIVE>::getProgressNormalized() const
{
    if (m_framesToWait == 0)
        return 0;

    return (float)m_currentFrame / m_framesToWait;
}

template <bool RESULT_IF_INACTIVE>
void FrameTimer<RESULT_IF_INACTIVE>::forceOver()
{
    m_framesToWait = 1;
    m_currentFrame = 1;
}

template class FrameTimer<true>;
template class FrameTimer<false>;

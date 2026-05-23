#include "FrameTimer.h"

template<bool RESULT_IF_INACTIVE>
FrameTimer<RESULT_IF_INACTIVE>::FrameTimer(uint32_t framesToWait_) noexcept
{
    begin(framesToWait_);
}

template<bool RESULT_IF_INACTIVE>
void FrameTimer<RESULT_IF_INACTIVE>::begin(uint32_t framesToWait_) noexcept
{
    m_currentFrame = 0;
    m_framesToWait = framesToWait_;
}

template <bool RESULT_IF_INACTIVE>
void FrameTimer<RESULT_IF_INACTIVE>::beginAt(uint32_t framesToWait_, float progress_) noexcept
{
    m_currentFrame = static_cast<uint32_t>(framesToWait_ * progress_);
    m_framesToWait = framesToWait_;
}

template <bool RESULT_IF_INACTIVE>
void FrameTimer<RESULT_IF_INACTIVE>::setCurrentFrame(uint32_t frame_) noexcept
{
    m_currentFrame = frame_;
}

template <bool RESULT_IF_INACTIVE>
uint32_t FrameTimer<RESULT_IF_INACTIVE>::getDuration() const noexcept
{
    return m_framesToWait;
}

template<bool RESULT_IF_INACTIVE>
bool FrameTimer<RESULT_IF_INACTIVE>::update() noexcept
{
    if (m_framesToWait == 0)
        return RESULT_IF_INACTIVE;

    if (m_currentFrame < m_framesToWait)
        m_currentFrame++;

    return m_currentFrame >= m_framesToWait;
}

template<bool RESULT_IF_INACTIVE>
bool FrameTimer<RESULT_IF_INACTIVE>::isOver() const noexcept
{
    if (m_framesToWait == 0)
        return RESULT_IF_INACTIVE;

    return m_currentFrame >= m_framesToWait;
}

template<bool RESULT_IF_INACTIVE>
bool FrameTimer<RESULT_IF_INACTIVE>::isActive() const noexcept
{
    if (m_framesToWait == 0)
        return false;

    return m_currentFrame < m_framesToWait;
}

template<bool RESULT_IF_INACTIVE>
uint32_t FrameTimer<RESULT_IF_INACTIVE>::getCurrentFrame() const noexcept
{
    return m_currentFrame;
}

template<bool RESULT_IF_INACTIVE>
float FrameTimer<RESULT_IF_INACTIVE>::getProgressNormalized() const noexcept
{
    if (m_framesToWait == 0)
        return 0;

    return (float)m_currentFrame / m_framesToWait;
}

template <bool RESULT_IF_INACTIVE>
void FrameTimer<RESULT_IF_INACTIVE>::finish() noexcept
{
    m_currentFrame = m_framesToWait;
}

template class FrameTimer<true>;
template class FrameTimer<false>;

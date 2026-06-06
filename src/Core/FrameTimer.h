#pragma once
#include <cstdint>

template<bool RESULT_IF_INACTIVE = false>
class FrameTimer
{
public:
	FrameTimer() noexcept = default;
	FrameTimer(uint32_t framesToWait_) noexcept;
	void begin(uint32_t framesToWait_) noexcept;
	void beginAt(uint32_t framesToWait_, float progress_) noexcept;
	void setCurrentFrame(uint32_t frame_) noexcept;
	uint32_t getDuration() const noexcept;
    bool update() noexcept;
	bool isOver() const noexcept;
	bool isActive() const noexcept;
	uint32_t getCurrentFrame() const noexcept;
	float getProgressNormalized() const noexcept;
	void finish() noexcept;

private:
	uint32_t m_framesToWait = 0;
    uint32_t m_currentFrame = 0;
};

#ifndef FRAME_TIMER_H_
#define FRAME_TIMER_H_
#include <cstdint>

template<bool RESULT_IF_INACTIVE = false>
class FrameTimer
{
public:
	FrameTimer(uint32_t framesToWait_ = 0);
	void begin(uint32_t framesToWait_);
	void beginAt(uint32_t framesToWait_, float progress_);
    bool update();
	bool isOver() const;
	bool isActive() const;
	uint32_t getCurrentFrame() const;
	float getProgressNormalized() const;

private:
	uint32_t m_framesToWait;
    uint32_t m_currentFrame;
};

#endif
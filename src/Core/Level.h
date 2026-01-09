#ifndef LEVEL_H_
#define LEVEL_H_

#include "InputSystem.h"
#include "Timer.h"
#include "Vector2.hpp"

class Application;

struct LevelResult
{
	int nextLvl;
};

class Level : public InputReactor
{
public:
    Level(const Vector2<int> &size_, int lvlId_);

    virtual void enter();
	virtual void leave();
    LevelResult proceed();

    void receiveEvents(GAMEPLAY_EVENTS event, const float scale_) override;

    virtual ~Level() = default;

protected:

    virtual void update() = 0;
	virtual void draw() const = 0;

    const Vector2<int> m_size;
    int m_levelId;
    LevelResult m_returnVal;
    enum class STATE : uint8_t
    {
        ENTER,
        RUNNING,
        LEAVE
    } m_state;
    Timer m_frameTimer;

    uint64_t m_timeForFrame;
    uint64_t m_lastFrameTimeNS;
    uint64_t m_lastFullFrameTime;
    bool m_globalPause = false;
    bool m_allowIter = false;
    bool m_forcerun = false;
};

#endif
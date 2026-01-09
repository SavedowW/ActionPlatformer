#ifndef LEVEL_H_
#define LEVEL_H_

#include "InputSystem.h"
#include "FPSUtility.h"
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
    Level(int lvlId_, FPSUtility &fpsUtility_, const Vector2<int> &size_);

    virtual void enter();
	virtual void leave();
    LevelResult proceed();

    void receiveEvents(GAMEPLAY_EVENTS event, const float scale_) override;

    virtual ~Level() = default;

protected:

    virtual void update() = 0;
	virtual void draw() const = 0;

    const Vector2<int> m_size;
    const int m_levelId;
    FPSUtility &m_fpsUtility;

    LevelResult m_returnVal;
    enum class STATE : uint8_t
    {
        ENTER,
        RUNNING,
        LEAVE
    } m_state;

    bool m_globalPause = false;
    bool m_allowIter = false;
    bool m_forcerun = false;
};

#endif
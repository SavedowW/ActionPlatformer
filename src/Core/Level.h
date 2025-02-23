#ifndef LEVEL_H_
#define LEVEL_H_

#include <memory>
#include "InputSystem.h"
#include "Timer.h"
#include "Vector2.h"

class Application;

struct LevelResult
{
	int nextLvl;
};

enum class MenuLevels { MAINMENU, NONE };

class Level : public InputReactor
{
public:
    Level(Application *application_, const Vector2<int> &size_, int lvlId_);

    virtual void enter();
	virtual void leave();
    LevelResult proceed();

    void receiveEvents(GAMEPLAY_EVENTS event, const float scale_) override;

    virtual ~Level() = default;

protected:

    virtual void update() = 0;
	virtual void draw() = 0;

    const Vector2<int> m_size;
    int m_levelId;
    LevelResult m_returnVal;
    enum class STATE {ENTER, RUNNING, LEAVE} m_state;
    Timer m_frameTimer;
    Application *m_application;

    nanoseconds m_timeForFrame;
    nanoseconds m_lastFrameTimeMS;
    nanoseconds m_lastFullFrameTime;
    bool m_globalPause = false;
    bool m_allowIter = false;
    bool m_forcerun = false;
};

#endif
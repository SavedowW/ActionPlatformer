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
    Level(Application *application_, const Vector2<float> &size_, int lvlId_);

    virtual void enter();
	virtual void leave();
    LevelResult proceed();

    void receiveInput(EVENTS event, const float scale_) override;

    virtual ~Level() = default;

protected:

    virtual void update() = 0;
	virtual void draw() = 0;

    Vector2<float> m_size;
    int m_levelId;
    LevelResult m_returnVal;
    enum class STATE {ENTER, RUNNING, LEAVE} m_state;
    Timer m_frameTimer;
    Application *m_application;
    float m_timeForFrame;

    bool m_globalPause = false;
    Uint32 m_lastFrameTimeMS = 0;
};

#endif
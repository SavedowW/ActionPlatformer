#pragma once
#include "InputSystem.h"
#include "FPSUtility.h"
#include "Timer.h"
#include "Vector2.hpp"

class Application;

struct LevelResult
{
	std::optional<std::string> nextLvl;
};

class Level : public InputReactor
{
public:
    Level(std::string levelName_, FPSUtility &fpsUtility_, const Vector2<int> &size_);

    virtual void enter();
    LevelResult proceed();
	virtual void leave();

    std::string name() const;

    void receiveEvents(GAMEPLAY_EVENTS event, float scale_) override;

    virtual ~Level() = default;

protected:
    virtual void update() = 0;
	virtual void draw() const = 0;

    const std::string m_levelName;
    FPSUtility &m_fpsUtility;
    const Vector2<int> m_size;

    LevelResult m_returnVal;
    enum class STATE : uint8_t
    {
        ENTER,
        RUNNING,
        LEAVE
    } m_state = STATE::ENTER;

    bool m_globalPause = false;
    bool m_allowIter = false;
    bool m_forcerun = false;
};

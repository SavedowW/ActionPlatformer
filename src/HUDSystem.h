#ifndef HUD_SYSTEM_H_
#define HUD_SYSTEM_H_
#include "Application.h"
#include "ImmediateScreenLog.h"
#include "CommonAI.h"
#include <entt/entt.hpp>

struct HudSystem
{
public:
    HudSystem(entt::registry &reg_, Application &app_, Camera &cam_, int lvlId_, const Vector2<float> lvlSize_, nanoseconds &frameTime_);

    void draw();
    void drawCommonDebug();
    void drawPlayerDebug();
    void drawNPCDebug(ComponentTransform &trans_, ComponentPhysical &phys_, StateMachine &sm_, ComponentAI &ai_);

    entt::entity m_playerId;

private:
    Renderer &m_renderer;
    TextManager &m_textManager;
    entt::registry &m_reg;

    Camera &m_cam;
    int m_lvlId;
    Vector2<float> m_lvlSize;
    nanoseconds &m_frameTime;

    ImmediateScreenLog m_commonLog;
    ImmediateScreenLog m_playerLog;

    Texture_t m_arrowIn;
    Texture_t m_arrowOut;
};

#endif

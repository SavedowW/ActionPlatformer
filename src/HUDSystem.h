#ifndef HUD_SYSTEM_H_
#define HUD_SYSTEM_H_
#include "Application.h"
#include "Texture.h"
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
    void drawNPCDebug(const ComponentTransform &trans_, const ComponentPhysical &phys_, const StateMachine &sm_, const ComponentAI &ai_);

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

    std::shared_ptr<Texture> m_arrowIn;
    std::shared_ptr<Texture> m_arrowOut;
};

#endif

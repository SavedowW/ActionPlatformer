#include "HUDSystem.h"
#include "CoreComponents.h"
#include "StateMachine.h"

HudSystem::HudSystem(entt::registry &reg_, Application &app_, Camera &cam_, int lvlId_, const Vector2<float> lvlSize_, Uint32 &frameTime_) :
    m_renderer(*app_.getRenderer()),
    m_textManager(*app_.getTextManager()),
    m_reg(reg_),
    m_cam(cam_),
    m_lvlId(lvlId_),
    m_lvlSize(lvlSize_),
    m_frameTime(frameTime_),
    m_commonLog(app_, 0, fonts::HOR_ALIGN::LEFT, 22),
    m_playerLog(app_, 0, fonts::HOR_ALIGN::RIGHT, 22)
{
    auto &texman = *app_.getTextureManager();

    m_arrowIn = texman.getTexture(texman.getTexID("UI/Arrow2"));
    m_arrowOut = texman.getTexture(texman.getTexID("UI/Arrow1"));
}

void HudSystem::draw()
{
    m_renderer.switchToHUD({0, 0, 0, 0});
    drawCommonDebug();
    drawPlayerDebug();
}

void HudSystem::drawCommonDebug()
{
    m_commonLog.addRecord("[" + std::to_string(m_lvlId) + "] " + utils::toString(m_lvlSize));
    m_commonLog.addRecord("Camera pos: " + utils::toString(m_cam.getPos()));
    m_commonLog.addRecord("Camera size: " + utils::toString(m_cam.getSize()));
    m_commonLog.addRecord("Camera scale: " + std::to_string(m_cam.getScale()));
    m_commonLog.addRecord("Real frame time (MS): " + std::to_string(m_frameTime));
    m_commonLog.addRecord("UTF-8: Кириллица работает");

    m_commonLog.dump({1.0f, 1.0f});
}

void HudSystem::drawPlayerDebug()
{
    auto &obsfall = m_reg.get<ComponentObstacleFallthrough>(m_playerId);
    auto &ptransform = m_reg.get<ComponentTransform>(m_playerId);
    auto &pphysical = m_reg.get<ComponentPhysical>(m_playerId);
    auto &psm = m_reg.get<StateMachine>(m_playerId);
    auto &pinp = m_reg.get<ComponentPlayerInput>(m_playerId);

    std::string ignoredObstacles = "";
    for (const auto &el : obsfall.m_ignoredObstacles)
        ignoredObstacles += std::to_string(el) + " ";

    //std::string cooldowns = "";
    //for (const auto &el : m_pc->m_cooldowns)
    //    cooldowns += std::to_string(!el.isActive());

    m_playerLog.addRecord("Player pos: " + utils::toString(ptransform.m_pos));
    m_playerLog.addRecord("Player vel: " + utils::toString(pphysical.m_velocity));
    m_playerLog.addRecord("Player inr: " + utils::toString(pphysical.m_inertia));
    m_playerLog.addRecord(std::string("Player action: ") + psm.getName());
    m_playerLog.addRecord(std::string("Ignored obstacles: ") + ignoredObstacles);
    m_playerLog.addRecord(std::string("On slope: ") + std::to_string(pphysical.m_onSlopeWithAngle));

    m_playerLog.dump({gamedata::global::defaultWindowResolution.x - 1.0f, 1.0f});

    auto inputs = pinp.m_inputResolver->getCurrentInputDir();

    Vector2<float> arrowPos[] = {
        Vector2{620.0f, 20.0f},
        Vector2{655.0f, 55.0f},
        Vector2{620.0f, 90.0f},
        Vector2{585.0f, 55.0f},
    };

    bool isValid[] = {
        inputs.y < 0,
        inputs.x > 0,
        inputs.y > 0,
        inputs.x < 0
    };

    float angles[] = {
        270,
        0,
        90,
        180
    };

    for (int i = 0; i < 4; ++i)
    {
        auto &spr = (isValid[i] ? m_arrowIn : m_arrowOut);
        SDL_FPoint sdlcenter = {spr->m_w / 2.0f, spr->m_h / 2.0f};

        m_renderer.renderTexture(spr->getSprite(),
        arrowPos[i].x, arrowPos[i].y, spr->m_w, spr->m_h, angles[i], &sdlcenter, SDL_FLIP_NONE);
    }
}

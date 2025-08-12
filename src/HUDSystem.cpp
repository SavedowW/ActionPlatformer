#include "HUDSystem.h"
#include "CoreComponents.h"
#include "StateMachine.h"
#include "GameData.h"
#include "Localization/LocalizationGen.h"
#include "Configuration.h"

HudSystem::HudSystem(entt::registry &reg_, Application &app_, Camera &cam_, int lvlId_, const Vector2<float> lvlSize_, nanoseconds &frameTime_) :
    m_renderer(app_.getRenderer()),
    m_textManager(app_.getTextManager()),
    m_reg(reg_),
    m_cam(cam_),
    m_lvlId(lvlId_),
    m_lvlSize(lvlSize_),
    m_frameTime(frameTime_),
    m_commonLog(app_, 0, fonts::HOR_ALIGN::LEFT, 12),
    m_playerLog(app_, 0, fonts::HOR_ALIGN::RIGHT, 12)
{
    auto &texman = app_.getTextureManager();

    m_arrowIn = texman.getTexture(texman.getTexID("UI/Arrow2"));
    m_arrowOut = texman.getTexture(texman.getTexID("UI/Arrow1"));
}

void HudSystem::draw()
{
    const auto npcs = m_reg.view<ComponentTransform, ComponentPhysical, StateMachine, ComponentAI>();

    m_renderer.switchToHUD({0, 0, 0, 0});
    drawCommonDebug();
    drawPlayerDebug();

    if (ConfigurationManager::instance().m_debug.m_drawNpcDebug)
    {
        for (const auto [idx, trans, phys, sm, ai] : npcs.each())
        {
            drawNPCDebug(trans, phys, sm, ai);
        }
    }
}

void HudSystem::drawCommonDebug()
{
    static bool firstRun = true;

    if (firstRun)
        firstRun = false;
    else
        m_avgFrames.push(static_cast<float>(m_frameTime.count()) / 1000000.0f);

    m_commonLog.addRecord("[" + std::to_string(m_lvlId) + "] " + utils::toString(m_lvlSize));
    m_commonLog.addRecord("Camera pos: " + utils::toString(m_cam.getPos()));
    m_commonLog.addRecord("Camera size: " + utils::toString(m_cam.getSize()));
    m_commonLog.addRecord("Camera scale: " + std::to_string(m_cam.getScale()));
    m_commonLog.addRecord("Real frame time (ns): " + std::to_string(m_frameTime.count()));
    m_commonLog.addRecord("Avg frame time (ms): " + std::to_string( m_avgFrames.avg()));
    m_commonLog.addRecord("FPS: " + std::to_string( 1000000000.0f / m_frameTime.count()));
    m_commonLog.addRecord("Avg FPS: " + std::to_string( 1000.0f / m_avgFrames.avg()));
    m_commonLog.addRecord("UTF-8: Кириллица работает");
    m_commonLog.addRecord(ll::dbg_localization());

    m_commonLog.dump({1.0f, 1.0f});
}

void HudSystem::drawPlayerDebug()
{
    const auto &obsfall = m_reg.get<ComponentObstacleFallthrough>(m_playerId);
    const auto &ptransform = m_reg.get<ComponentTransform>(m_playerId);
    const auto &pphysical = m_reg.get<ComponentPhysical>(m_playerId);
    const auto &psm = m_reg.get<StateMachine>(m_playerId);
    const auto &pinp = m_reg.get<ComponentPlayerInput>(m_playerId);

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
    m_playerLog.addRecord(std::string("Grounded: ") + std::to_string(pphysical.m_onGround != entt::null));
    m_playerLog.addRecord(std::string("Attac\nhed: ") + std::to_string(pphysical.m_onWall != entt::null));

    m_playerLog.dump({gamedata::global::hudLayerResolution.x - 1.0f, 1.0f});

    auto inputs = pinp.m_inputResolver->getCurrentInputDir();

    Vector2<float> arrowPos[] = {
        Vector2{320.0f, 25.0f},
        Vector2{355.0f, 60.0f},
        Vector2{320.0f, 95.0f},
        Vector2{285.0f, 60.0f},
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

        m_renderer.renderTexture(spr->m_id,
        arrowPos[i] - spr->m_size / 2, spr->m_size,SDL_FLIP_NONE, angles[i], spr->m_size / 2);
    }
}

void HudSystem::drawNPCDebug(const ComponentTransform &trans_, const ComponentPhysical &phys_, const StateMachine &sm_, const ComponentAI &ai_)
{
    auto txt1 = sm_.getName();
    auto txt2 = ai_.m_sm.getName();
    auto worldOrigin = trans_.m_pos + phys_.m_pushbox.m_topLeft + Vector2{phys_.m_pushbox.m_size.x, 0};

    Vector2<int> camSize = m_cam.getSize();
    Vector2<int> camTL = m_cam.getTopLeft();
    auto screenRelPos = (worldOrigin - camTL).mulComponents(1.0f / camSize.x, 1.0f / camSize.y);
    
	Vector2<int> screenOrigin = screenRelPos.mulComponents(gamedata::global::hudLayerResolution);

    m_textManager.renderText(txt1, 1, screenOrigin);
    m_textManager.renderText(txt2, 1, screenOrigin + Vector2{0.0f, 10.0f});
}

#include "HUDSystem.h"
#include "Core/TextManager.hpp"
#include "PlayableCharacter.h"
#include "SM/StateMachine.h"
#include "Core/ImmediateScreenLog.hpp"
#include "Core/Application.h"
#include "Core/CoreComponents.h"
#include "Core/InputResolver.h"
#include "Core/GameData.h"
#include "Core/Localization/LocalizationGen.h"
#include "Core/Configuration.h"

HudSystem::HudSystem(entt::registry &reg_, Camera &cam_, int lvlId_, const Vector2<float> &lvlSize_) :
    m_renderer(Application::instance().m_renderer),
    m_window{Application::instance().m_window},
    m_textManager(Application::instance().m_textManager),
    m_reg(reg_),
    m_cam(cam_),
    m_lvlId(lvlId_),
    m_lvlSize(lvlSize_)
{
    auto &texman = Application::instance().m_textureManager;

    m_arrowIn = texman.getTexture(texman.getTexID("UI/Arrow2"));
    m_arrowOut = texman.getTexture(texman.getTexID("UI/Arrow1"));
}

void HudSystem::draw() const
{
    const auto npcs = m_reg.view<ComponentTransform, ComponentPhysical/*, StateMachine*/, ComponentAI>();

    drawCommonDebug();
    drawPlayerDebug();

    if (ConfigurationManager::instance().m_debug.m_drawNpcDebug)
    {
        for (const auto [idx, trans, phys/*, sm*/, ai] : npcs.each())
        {
            drawNPCDebug(trans, phys/*, sm*/, ai);
        }
    }
}

void HudSystem::drawCommonDebug() const 
{
    static bool firstRun = true;

    const auto &lastCycleCalls = Application::instance().getFPSUtility().lastCycleCalls;
    const auto lastFrameTime = lastCycleCalls[0] - lastCycleCalls[1];

    if (firstRun)
        firstRun = false;
    else
        m_avgFrames.push(static_cast<float>(lastFrameTime));

    ImmediateScreenLog<TextAligners::AlignerLeft> commonLog{0, 36, {1, 1}};

    commonLog.dumpLine("[" + std::to_string(m_lvlId) + "] " + std::string(m_lvlSize));
    commonLog.dumpLine("Camera pos: " + std::string(m_cam.getPos()));
    commonLog.dumpLine("Camera size: " + std::string(m_cam.getSize()));
    commonLog.dumpLine("Camera scale: " + std::to_string(m_cam.getScale()));
    commonLog.dumpLine("Real frame time (ns): " + std::to_string(lastFrameTime));
    commonLog.dumpLine("Avg frame time (ms): " + std::to_string( m_avgFrames.avg() / 1'000'000.0f));
    commonLog.dumpLine("FPS: " + std::to_string(1'000'000'000.0f / static_cast<float>(lastFrameTime)));
    commonLog.dumpLine("Avg FPS: " + std::to_string( 1'000'000'000.0f / m_avgFrames.avg()));
    commonLog.dumpLine("UTF-8: Кириллица работает");
    commonLog.dumpLine(ll::dbg_localization());
}

void HudSystem::drawPlayerDebug() const
{
    const auto &obsfall = m_reg.get<ComponentObstacleFallthrough>(playerId);
    const auto &ptransform = m_reg.get<ComponentTransform>(playerId);
    const auto &pphysical = m_reg.get<ComponentPhysical>(playerId);
    const auto &psm = m_reg.get<SM::StatePossessor<PlayerState>>(playerId);
    const auto &pinp = m_reg.get<InputResolver>(playerId);

    std::string ignoredObstacles;
    for (const auto &el : obsfall.m_ignoredObstacles)
        ignoredObstacles += std::to_string(el) + " ";

    //std::string cooldowns = "";
    //for (const auto &el : m_pc->m_cooldowns)
    //    cooldowns += std::to_string(!el.isActive());

    const auto resolution = m_window.getResolution();

    ImmediateScreenLog<TextAligners::AlignerRight> playerLog{0, 36, {resolution.x - 1, 1}};

    playerLog.dumpLine("Player pos: " + std::string(ptransform.m_pos));
    playerLog.dumpLine("Player vel: " + std::string(pphysical.m_velocity));
    playerLog.dumpLine("Player inr: " + std::string(pphysical.m_inertia));
    playerLog.dumpLine("Gravity: " + std::string(pphysical.m_gravity));
    playerLog.dumpLine(std::string("Player action: ") + serialize(psm.stateId()) + ':' + std::to_string(psm.framesInState()));
    playerLog.dumpLine(std::string("Ignored obstacles: ") + ignoredObstacles);
    playerLog.dumpLine(std::string("On slope: ") + std::to_string(pphysical.m_onSlopeWithAngle));
    playerLog.dumpLine(std::string("Grounded: ") + std::to_string(pphysical.m_onGround != entt::null));
    playerLog.dumpLine(std::string("Attached: ") + std::to_string(pphysical.m_onWall != entt::null));

    auto inputs = pinp.getCurrentInputDir();

    const std::array<Vector2<int>, 4> arrowPos {
        Vector2{resolution.x / 2, 25},
        Vector2{resolution.x / 2 + 35, 60},
        Vector2{resolution.x / 2, 95},
        Vector2{resolution.x / 2 - 35, 60},
    };

    const std::array<bool, 4> isValid {
        inputs.y < 0,
        inputs.x > 0,
        inputs.y > 0,
        inputs.x < 0
    };

    const std::array<float, 4> angles {
        270,
        0,
        90,
        180
    };

    for (int i = 0; i < 4; ++i)
    {
        const auto &spr = (isValid.at(i) ? *m_arrowIn : *m_arrowOut);

        m_renderer.renderTexture(spr.handler(),
        arrowPos.at(i) - spr.size() / 2, spr.size(), SDL_FLIP_NONE, angles.at(i), spr.size() / 2);
    }
}

void HudSystem::drawNPCDebug(const ComponentTransform &trans_, const ComponentPhysical &phys_, /*const StateMachine &sm_, */const ComponentAI &) const
{
    const auto txt1 = std::string("SM name used to be here, TODO"); //sm_.getName();
    const auto txt2 = std::string("AI SM name used to be here, TODO");
    const auto worldOrigin = trans_.m_pos + phys_.m_pushbox.m_topLeft + Vector2{phys_.m_pushbox.m_size.x, 0};

    const Vector2<int> camSize = m_cam.getSize();
    const Vector2<int> camTL = m_cam.getTopLeft();
    const auto screenRelPos = (worldOrigin - camTL).mulComponents(1.0f / camSize.x, 1.0f / camSize.y);
    
	const Vector2<int> screenOrigin = screenRelPos.mulComponents(m_window.getResolution());

    m_textManager.renderText<TextAligners::AlignerLeft>(txt1, 1, screenOrigin);
    m_textManager.renderText<TextAligners::AlignerLeft>(txt2, 1, screenOrigin + Vector2{0.0f, 10.0f});
}

#include "HUDSystem.h"
#include "Core/SlidingWindow.hpp"
#include "Core/TextManager.hpp"
#include "Core/Utils.hpp"
#include "PlayableCharacter.h"
#include "SM/StateMachine.h"
#include "Core/ImmediateScreenLog.hpp" // IWYU pragma: keep
#include "Core/Application.h"
#include "Core/CoreComponents.h"
#include "Core/InputResolver.h"
#include "Core/Localization/LocalizationGen.h"
#include "Core/Configuration.h"

HudSystem::HudSystem(entt::registry &reg_, Camera &cam_, std::string levelname_, const Vector2<float> &lvlSize_, const PlayerSystem &playersys_) :
    m_renderer(Application::instance().m_renderer),
    m_window{Application::instance().m_window},
    m_textManager(Application::instance().m_textManager),
    m_reg{reg_},
    m_playersys{playersys_},
    m_cam{cam_},
    m_levelname{std::move(levelname_)},
    m_lvlSize{lvlSize_}
{
    auto &texman = Application::instance().m_textureManager;

    m_arrowIn = texman.getTexture(texman.getTexID("UI/Arrow2"));
    m_arrowOut = texman.getTexture(texman.getTexID("UI/Arrow1"));
}

void HudSystem::draw() const
{
#if 0
    const auto npcs = m_reg.view<ComponentTransform, ComponentPhysical/*, StateMachine*/, ComponentAI>();
#endif

    drawCommonDebug();

    const auto playerId = m_playersys.getPlayerId();
    if (playerId != entt::null)
        drawPlayerDebug(playerId);

    if (ConfigurationManager::instance().m_debug.m_drawNpcDebug)
    {
#if 0
        for (const auto [idx, trans, phys/*, sm*/, ai] : npcs.each())
        {
            drawNPCDebug(trans, phys/*, sm*/, ai);
        }
#endif
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

    ImmediateScreenLog<TextAligners::AlignerLeft> commonLog{Fonts::DBG_UI, 32, {1, 1}};

    commonLog.dumpLine(std::format("[\"{}\"] {}", m_levelname, m_lvlSize));
    commonLog.dumpLine(std::format("Camera pos: ", m_cam.getPos()));
    commonLog.dumpLine(std::format("Camera size: ", m_cam.getSize()));
    commonLog.dumpLine(std::format("Camera scale: ", m_cam.getScale()));
    commonLog.dumpLine(std::format("Real frame time (ns): {}", lastFrameTime));
    commonLog.dumpLine(std::format("Avg frame time (ms): {}", m_avgFrames.avg() / 1'000'000.0f));
    commonLog.dumpLine(std::format("FPS: {}", 1'000'000'000.0f / static_cast<float>(lastFrameTime)));
    commonLog.dumpLine(std::format("Avg FPS: ", 1'000'000'000.0f / m_avgFrames.avg()));
    commonLog.dumpLine("UTF-8: Кириллица работает");
    commonLog.dumpLine(ll::dbg_localization());
}

void HudSystem::drawPlayerDebug(entt::entity playerId_) const
{
    const auto &obsfall = m_reg.get<ComponentObstacleFallthrough>(playerId_);
    const auto &ptransform = m_reg.get<ComponentTransform>(playerId_);
    const auto &pphysical = m_reg.get<ComponentPhysical>(playerId_);
    const auto &worldPos = m_reg.get<WorldPosition>(playerId_);
    const auto &psm = m_reg.get<SM::StatePossessor<PlayerState>>(playerId_);
    const auto &pinp = m_reg.get<InputResolver>(playerId_);

    std::string ignoredObstacles;
    for (const auto &el : obsfall.m_ignoredObstacles)
        ignoredObstacles += std::to_string(static_cast<uint32_t>(el)) + " ";

    //std::string cooldowns = "";
    //for (const auto &el : m_pc->m_cooldowns)
    //    cooldowns += std::to_string(!el.isActive());

    const auto resolution = m_window.getResolution();

    ImmediateScreenLog<TextAligners::AlignerRight> playerLog{Fonts::DBG_UI, 32, {resolution.x - 1, 1}};

    playerLog.dumpLine(std::format("Player pos: {}", ptransform.m_pos));
    playerLog.dumpLine(std::format("Player vel: {}", pphysical.velocity));
    playerLog.dumpLine(std::format("Player inr: {}", pphysical.inertia));
    playerLog.dumpLine(std::format("Gravity: {}", pphysical.gravity));
    playerLog.dumpLine(std::format("Player action: {}:{}", serialize(psm.stateId()), psm.framesInState()));
    playerLog.dumpLine(std::format("Ignored obstacles: {}", ignoredObstacles));
    playerLog.dumpLine(std::format("On slope: {}", worldPos.ground.onSlopeWithAngle));
    playerLog.dumpLine(std::format("Grounded: {}", worldPos.ground.onGround != entt::null));
    playerLog.dumpLine(std::format("Has wall: {}-{}", worldPos.wall.leftWall != entt::null, worldPos.wall.rightWall != entt::null));

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

void HudSystem::drawNPCDebug(const ComponentTransform &trans_, const ComponentPhysical &phys_) const
{
    const auto txt1 = std::string("SM name used to be here, TODO"); //sm_.getName();
    const auto txt2 = std::string("AI SM name used to be here, TODO");
    const auto worldOrigin = trans_.m_pos + phys_.pushbox.m_topLeft + Vector2{phys_.pushbox.m_size.x, 0};

    const Vector2<int> camSize = m_cam.getSize();
    const Vector2<int> camTL = m_cam.getTopLeft();
    const auto screenRelPos = (worldOrigin - camTL).mulComponents(1.0f / camSize.x, 1.0f / camSize.y);
    
	const Vector2<int> screenOrigin = screenRelPos.mulComponents(m_window.getResolution());

    m_textManager.renderText<TextAligners::AlignerLeft>(txt1, Fonts::DBG_NPC, screenOrigin);
    m_textManager.renderText<TextAligners::AlignerLeft>(txt2, Fonts::DBG_NPC, screenOrigin + Vector2{0.0f, 10.0f});
}

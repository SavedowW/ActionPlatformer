#include "BattleLevel.h"
#include "SM/StateMachine.hpp"
#include "World.h"
#include "ResetHandlers.h"
#include "Core/GameData.h"
#include "Core/Application.h"
#include "Core/InputResolver.h"
#include "Core/Localization/LocalizationGen.h"
#include "Core/Profile.h"

BattleLevel::BattleLevel(int lvlId_, FPSUtility &fpsUtility_, const Vector2<int>& size_) :
    Level(lvlId_, fpsUtility_, size_),
    m_camera({0.0f, 0.0f}, gamedata::global::maxCameraSize, m_size),
    m_playerSystem(m_registry),
    m_rendersys(m_registry, m_camera, m_cldRoutesCollection),
    m_inputsys(m_registry),
    m_physsys(m_registry, size_),
    m_camsys(m_registry, m_camera),
    m_hudsys(m_registry, m_camera, lvlId_, size_),
    m_enemysys(m_registry, m_navsys, m_camera, m_partsys),
    m_aisys(m_registry),
    m_navsys(m_registry, m_graph),
    m_colsys(m_registry),
    m_partsys(m_registry),
    m_battlesys(m_registry, m_camera),
    m_chatBoxSys(m_registry, m_camera, m_inputsys),
    m_envSystem(m_registry),
    m_lvlBuilder(m_registry)
{
    auto playerId = m_registry.create();
    m_registry.emplace<ComponentName>(playerId, "Player");

    const auto &ptrans = m_registry.emplace<ComponentTransform>(playerId);
    m_registry.emplace<ComponentReset<ComponentTransform>>(playerId, ptrans.m_pos, ptrans.m_orientation);

    m_registry.emplace<ComponentPhysical>(playerId);
    m_registry.emplace<ComponentResetStatic<ComponentPhysical>>(playerId);

    m_registry.emplace<ComponentObstacleFallthrough>(playerId);
    
    m_registry.emplace<ComponentAnimationRenderable>(playerId).m_drawOutline = true;
    m_registry.emplace<ComponentResetStatic<ComponentAnimationRenderable>>(playerId);

    m_registry.emplace<RenderLayer>(playerId, 6);
    m_registry.emplace<InputResolver>(playerId);

    m_registry.emplace<ComponentDynamicCameraTarget>(playerId);
    m_registry.emplace<ComponentResetStatic<ComponentDynamicCameraTarget>>(playerId);

    m_registry.emplace<World>(playerId, m_registry, m_camera, m_partsys, m_navsys);

    m_registry.emplace<SM::StatePossessor<PlayerState>>(playerId, PlayerState::FLOAT);
    // m_registry.emplace<StateMachine>(playerId); TODO:
    // m_registry.emplace<ComponentReset<StateMachine>>(playerId);

    m_registry.emplace<PhysicalEvents>(playerId);
    m_registry.emplace<BattleActor>(playerId, BattleTeams::PLAYER);
    m_registry.emplace<HUDPoint>(playerId, HUDPosRule::REL_TRANSFORM, Vector2{0, -16}, 16);
    m_registry.emplace<HealthOwner>(playerId, 3);
    m_registry.emplace<HealthRendererCommonWRT>(playerId, 3, Vector2{0.0f, -28.0f});
    
    

    m_playerId = playerId;
    m_camsys.playerId = playerId;
    m_hudsys.playerId = playerId;
    m_enemysys.m_playerId = playerId;

    //m_envSystem.makeGrassTop(Vector2{230, 351});
    
    m_lvlBuilder.buildLevel("Tilemaps/Level1.json", playerId, m_graph, m_cldRoutesCollection);
    
    m_chatBoxSys.setPlayerEntity(m_playerId);
    
    subscribe(GAMEPLAY_EVENTS::FN4);
    subscribe(GAMEPLAY_EVENTS::RESET_DBG);

//for (int i = 0; i < 1000; ++i)
    //m_enemyId = m_enemysys.makeEnemy();
}

void BattleLevel::enter()
{
    Level::enter();

    m_playerSystem.setup(m_playerId);

    m_camera.setScale(1.0f);
    m_camera.setPos({320.0f, 383.0f});
}

void BattleLevel::receiveEvents(GAMEPLAY_EVENTS event, const float scale_)
{
    switch (event)
    {
        case GAMEPLAY_EVENTS::FN4:
            if (scale_ > 0)
            {
                ChatMessageSequence seq{m_playerId, ChatBoxSide::PREFER_TOP, true, true, true, true};
                seq.addMessage({ll::test_dlg1(), 3});
                seq.addMessage({ll::test_dlg2(), 3});
                seq.addMessage({ll::test_dlg3(), 3});
                // TODO: doesn't work with first tech symbol
                //seq.m_messages.emplace_back("<shake=2, 2, 0.001>I believe I shall\nanswer your<charspd=8,default>...</shake> <charspd=default,default><shake=2,2, 0.5>request</shake>.", 3);
                m_chatBoxSys.addSequence(std::move(seq));

                ChatMessageSequence seq2{m_enemyId, ChatBoxSide::PREFER_BOTTOM, true, true, true, true};
                seq2.addMessage({ll::test_dlg4(), 3});
                seq2.addMessage({ll::test_dlg5(), 3});
                m_chatBoxSys.addSequence(std::move(seq2));
            }
            else
                Level::receiveEvents(event, scale_);
        break;

        case GAMEPLAY_EVENTS::RESET_DBG:
            if (scale_ > 0)
                handleReset();
            else
                Level::receiveEvents(event, scale_);
        break;

        default:
            Level::receiveEvents(event, scale_);
    }

}

void BattleLevel::update()
{
    PROFILE_FUNCTION;

    m_physsys.prepHitstop();

    m_inputsys.update();

    m_rendersys.update();

    m_navsys.update();

    m_aisys.update();

    m_playerSystem.update();

    m_physsys.prepEntities();

    m_colsys.updateSwitchingColliders();

    m_colsys.updateMovingColliders();

    m_partsys.update();

    m_physsys.updatePhysics();
    
    m_battlesys.update();
    m_battlesys.handleAttacks();

    m_envSystem.update(m_playerId);

    m_camsys.update();

    // Not actually an ECS system
    m_chatBoxSys.update();

    /*
        Just updates camera shake logic, but many systems can cause shake
    */
    m_camera.update();

    m_rendersys.updateDepth();
}

void BattleLevel::draw() const
{
    PROFILE_FUNCTION;

    auto &renderer = Application::instance().m_renderer;
    renderer.switchToWorld(gamedata::colors::LVL2);

    m_rendersys.draw();

    m_graph.draw(m_camera);
    m_navsys.draw(m_camera);

    m_camsys.debugDraw(renderer, m_camera);

    m_battlesys.debugDraw();

    renderer.switchToHUD({0, 0, 0, 0});

    m_chatBoxSys.draw();

    renderer.switchToDBG({0, 0, 0, 0});

    m_hudsys.draw();

    renderer.updateScreen(m_camera);
}

template <typename... Components>
inline void BattleLevel::handleResetStaticHandler()
{
    auto view = m_registry.view<Components..., ComponentResetStatic<Components...>>();

    view.each(&ComponentResetStatic<Components...>::resetComponent);
}

template <typename Component>
inline void BattleLevel::handleResetHandler()
{
    auto view = m_registry.view<Component, ComponentReset<Component>>();

    for (auto [idx, comp, handler] : view.each())
    {
        handler.resetComponent(idx, comp);
    }
}

/* template <> TODO:
inline void BattleLevel::handleResetHandler<StateMachine>()
{
    auto view = m_registry.view<StateMachine, ComponentReset<StateMachine>>();

    for (auto [idx, comp, handler] : view.each())
    {
        handler.resetComponent({.reg=&m_registry, .idx=idx}, comp);
    }
}*/

void BattleLevel::handleReset()
{
    std::cout << "Running reset" << std::endl;
    
    handleResetStaticHandler<MoveCollider2Points>();
    handleResetStaticHandler<ColliderRoutingIterator>();
    handleResetStaticHandler<ComponentPhysical>();
    handleResetStaticHandler<ComponentDynamicCameraTarget>();
    handleResetStaticHandler<ComponentAnimationRenderable>();

    handleResetHandler<ComponentTransform>();
    // handleResetHandler<StateMachine>(); TODO:
}

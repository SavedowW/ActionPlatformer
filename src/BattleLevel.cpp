#include "BattleLevel.h"
#include "TileMapHelper.hpp"
#include "World.h"

BattleLevel::BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_) :
    Level(application_, size_, lvlId_),
    m_camera({0.0f, 0.0f}, gamedata::global::baseResolution, m_size),
    m_decor(application_),
    m_tlmap(application_),
    m_playerSystem(m_registry, *application_),
    m_rendersys(m_registry, *application_, m_camera),
    m_inputsys(m_registry),
    m_physsys(m_registry, size_),
    m_camsys(m_registry, m_camera),
    m_hudsys(m_registry, *application_, m_camera, lvlId_, size_, m_lastFrameTimeMS),
    m_enemysys(m_registry, *application_),
    m_aisys(m_registry)
{
    auto playerId = m_registry.create();
    m_registry.emplace<ComponentTransform>(playerId, Vector2{100.0f, 300.0f}, ORIENTATION::RIGHT);
    m_registry.emplace<ComponentPhysical>(playerId);
    m_registry.emplace<ComponentObstacleFallthrough>(playerId);
    m_registry.emplace<ComponentAnimationRenderable>(playerId);
    m_registry.emplace<ComponentPlayerInput>(playerId, std::unique_ptr<InputResolver>(new InputResolver(application_->getInputSystem())));
    m_registry.emplace<ComponentDynamicCameraTarget>(playerId);
    m_registry.emplace<World>(playerId, m_registry, m_camera);
    m_registry.emplace<StateMachine>(playerId);

    auto nd1 = m_graph.makeNode({8.0f, 300.0f});
    auto nd2 = m_graph.makeNode({72.0f, 300.0f});
    m_graph.makeConnection(nd1, nd2, {0}, {0});


    m_playerId = playerId;
    m_camsys.m_playerId = playerId;
    m_hudsys.m_playerId = playerId;
    m_enemysys.m_playerId = playerId;

    m_enemysys.makeEnemy();

    m_tlmap.load("Tiles/tiles");

    LevelBuilder bld(*application_, m_registry);
    m_decor = std::move(bld.buildLevel("Resources/Sprites/Tiles/tilemap.json", m_tlmap, playerId));
}

void BattleLevel::enter()
{
    Level::enter();

    m_playerSystem.setup(m_playerId);

    m_camera.setScale(gamedata::global::minCameraScale);
    m_camera.setPos({0.0f, 0.0f});

    auto newfocus = m_registry.create();
    m_registry.emplace<CameraFocusArea>(newfocus, getTilePos(Vector2{53, 23}), gamedata::global::tileSize.mulComponents(Vector2{20.0f, 16.0f}), *m_application->getRenderer());
    m_registry.get<CameraFocusArea>(newfocus).overrideFocusArea({getTilePos(Vector2{53.0f, 24.0f}), gamedata::global::tileSize.mulComponents(Vector2{4.0f, 12.0f}) / 2.0f});
}

void BattleLevel::update()
{
    m_inputsys.update();
    m_playerSystem.update();
    m_aisys.update();
    m_physsys.update();
    m_camsys.update();

    m_camera.update();
}

void BattleLevel::draw()
{
    auto &renderer = *m_application->getRenderer();
    renderer.prepareRenderer(gamedata::colors::LVL2);

    m_decor.draw(m_camera);

    m_rendersys.draw();

    m_graph.draw(renderer, m_camera);

    m_camsys.debugDraw(renderer, m_camera);

    m_hudsys.draw();

    renderer.updateScreen(m_camera);
}

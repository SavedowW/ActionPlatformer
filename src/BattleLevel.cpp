#include "BattleLevel.h"
#include "TileMapHelper.hpp"
#include "World.h"

void addTrigger(entt::registry &reg_, const Trigger &trg_)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentTrigger>(newid, trg_);
}

void addCollider(entt::registry &reg_, const SlopeCollider &cld_, int id_ = 0)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentStaticCollider>(newid, cld_, id_);
}

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
    m_hudsys(m_registry, *application_, m_camera, lvlId_, size_, m_lastFrameTimeMS)
{
    auto playerId = m_registry.create();
    m_registry.emplace<ComponentTransform>(playerId, Vector2{50.0f, 300.0f}, ORIENTATION::RIGHT);
    m_registry.emplace<ComponentPhysical>(playerId);
    m_registry.emplace<ComponentObstacleFallthrough>(playerId);
    m_registry.emplace<ComponentAnimationRenderable>(playerId);
    m_registry.emplace<ComponentPlayerInput>(playerId, std::unique_ptr<InputResolver>(new InputResolver(application_->getInputSystem())));
    m_registry.emplace<ComponentDynamicCameraTarget>(playerId);
    m_registry.emplace<World>(playerId, m_registry, m_camera);
    m_registry.emplace<StateMachine>(playerId);

    m_camsys.m_playerId = playerId;
    m_hudsys.m_playerId = playerId;

    m_tlmap.load("Tiles/tiles");

    DecorationBuilder bld(*application_);
    m_decor = std::move(bld.buildDecorLayers("Resources/Sprites/Tiles/tilemap.json", m_tlmap));
}

void BattleLevel::enter()
{
    Level::enter();

    m_playerSystem.setup();

    m_camera.setScale(gamedata::global::minCameraScale);
    m_camera.setPos({0.0f, 0.0f});

    //addCollider(m_registry, getColliderForTileRange(Vector2{0, 20}, Vector2{6, 1}, 0));
    //addCollider(m_registry, getColliderForTileRange(Vector2{6, 20}, Vector2{1, 1}, -1), 1);

    addCollider(m_registry, getColliderForTileRange(Vector2{0, 30}, Vector2{60, 3}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{0, 26}, Vector2{6, 4}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{6, 26}, Vector2{4, 4}, 1));
    addCollider(m_registry, getColliderForTileRange(Vector2{19, 30}, Vector2{2, 0}, -1));
    addCollider(m_registry, getColliderForTileRange(Vector2{22, 28}, Vector2{2, 0}, -1));
    addCollider(m_registry, getColliderForTileRange(Vector2{21, 28}, Vector2{8, 2}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{24, 26}, Vector2{5, 2}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{29, 26}, Vector2{4, 4}, 1));
    addCollider(m_registry, getColliderForTileRange(Vector2{54, 7}, Vector2{6, 23}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{50, 7}, Vector2{4, 2}, 0));

    addCollider(m_registry, getColliderForTileRange(Vector2{36, 26}, Vector2{1, 3}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{39, 23}, Vector2{1, 3}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{40, 20}, Vector2{1, 2}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{42, 20}, Vector2{5, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{36, 16}, Vector2{14, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{51, 18}, Vector2{1, 10}, 0));

    addCollider(m_registry, getColliderForTileRange(Vector2{10, 22}, Vector2{11, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{6, 26}, Vector2{4, 0}, -1), 1);
    addCollider(m_registry, getColliderForTileRange(Vector2{21, 22}, Vector2{4, 4}, 1), 2);
    addCollider(m_registry, getColliderForTileRange(Vector2{52, 18}, Vector2{2, 1}, 0), 1);
    addCollider(m_registry, getColliderForTileRange(Vector2{34, 8}, Vector2{1, 8}, 0));


    auto newfocus = m_registry.create();
    m_registry.emplace<CameraFocusArea>(newfocus, getTilePos(Vector2{53, 23}), gamedata::global::tileSize.mulComponents(Vector2{20.0f, 16.0f}), *m_application->getRenderer());
    m_registry.get<CameraFocusArea>(newfocus).overrideFocusArea({getTilePos(Vector2{53.0f, 24.0f}), gamedata::global::tileSize.mulComponents(Vector2{4.0f, 12.0f}) / 2.0f});
}

void BattleLevel::update()
{
    m_inputsys.update();
    m_playerSystem.update();
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

    m_camsys.debugDraw(renderer, m_camera);

    m_hudsys.draw();

    renderer.updateScreen(m_camera);
}

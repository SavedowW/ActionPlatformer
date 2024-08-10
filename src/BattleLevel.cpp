#include "BattleLevel.h"
#include "TileMapHelper.hpp"
#include "World.h"

void addCollider(entt::registry &reg_, const SlopeCollider &cld_)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentStaticCollider>(newid, cld_, 0);
}

void addTrigger(entt::registry &reg_, const Trigger &trg_)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentTrigger>(newid, trg_);
}

void addCollider(entt::registry &reg_, const SlopeCollider &cld_, int id_)
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

    m_tlmap.load("Tiles/Tilemap-sheet");

    DecorationBuilder bld(*application_);
    m_decor = std::move(bld.buildDecorLayers("Resources/Sprites/Tiles/map.json", m_tlmap));
}

void BattleLevel::enter()
{
    Level::enter();

    m_playerSystem.setup();

    m_camera.setScale(gamedata::global::minCameraScale);
    m_camera.setPos({0.0f, 0.0f});

    addCollider(m_registry, getColliderForTileRange(Vector2{0, 20}, Vector2{6, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{6, 20}, Vector2{1, 1}, -1), 1);
    addCollider(m_registry, getColliderForTileRange(Vector2{7, 19}, Vector2{1, 1}, -1), 1);
    addCollider(m_registry, getColliderForTileRange(Vector2{8, 18}, Vector2{1, 1}, -1), 1);
    addCollider(m_registry, getColliderForTileRange(Vector2{9, 17}, Vector2{1, 1}, -1), 1);
    addCollider(m_registry, getColliderForTileRange(Vector2{10, 16}, Vector2{1, 1}, -1), 1);
    addCollider(m_registry, getColliderForTileRange(Vector2{11, 15}, Vector2{2, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{13, 15}, Vector2{1, 1}, -0.5));
    addCollider(m_registry, getColliderForTileRange(Vector2{14.0f, 14.5f}, Vector2{9, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{23.0f, 14.5f}, Vector2{1, 1}, 0.5));
    addCollider(m_registry, getColliderForTileRange(Vector2{24, 15}, Vector2{1, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{25, 15}, Vector2{6, 6}, 1), 1);

    addCollider(m_registry, getColliderForTileRange(Vector2{6, 20}, Vector2{1, 1}, 1));
    addCollider(m_registry, getColliderForTileRange(Vector2{7, 21}, Vector2{1, 1}, 1));
    addCollider(m_registry, getColliderForTileRange(Vector2{8, 22}, Vector2{1, 1}, 1));
    addCollider(m_registry, getColliderForTileRange(Vector2{9, 23}, Vector2{1, 1}, 1));
    addCollider(m_registry, getColliderForTileRange(Vector2{10, 24}, Vector2{1, 1}, 1));

    addCollider(m_registry, getColliderForTileRange(Vector2{11, 25}, Vector2{15, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{26, 25}, Vector2{1, 1}, -1));
    addCollider(m_registry, getColliderForTileRange(Vector2{27, 24}, Vector2{1, 1}, -1));
    addCollider(m_registry, getColliderForTileRange(Vector2{28, 23}, Vector2{1, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{29, 23}, Vector2{1, 1}, -1));
    addCollider(m_registry, getColliderForTileRange(Vector2{30, 22}, Vector2{1, 1}, -1));
    addCollider(m_registry, getColliderForTileRange(Vector2{31, 21}, Vector2{9, 9}, 1));
    addCollider(m_registry, getColliderForTileRange(Vector2{40, 30}, Vector2{5, 1}, 0));

    addCollider(m_registry, getColliderForTileRange(Vector2{45, 6}, Vector2{1, 25}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{40, 1}, Vector2{1, 23}, 0));

    addCollider(m_registry, getColliderForTileRange(Vector2{41, 6}, Vector2{5, 1}, 0), 3);

    addCollider(m_registry, getColliderForTileRange(Vector2{41, 0}, Vector2{10, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{50, 30}, Vector2{6, 1}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{69, 30}, Vector2{6, 1}, 0));

    addCollider(m_registry, getColliderForTileRange(Vector2{6, 4}, Vector2{1, 7}, 0));

    addCollider(m_registry, getColliderForTileRange(Vector2{1, 15}, Vector2{1, 3}, 0));
    addCollider(m_registry, getColliderForTileRange(Vector2{1, 9}, Vector2{1, 1}, 0));

    //addCollider(m_registry, getColliderForTileRange(Vector2{54, 25}, Vector2{1, 1}, 0));

    auto newfocus = m_registry.create();
    m_registry.emplace<CameraFocusArea>(newfocus, getTilePos(Vector2{43, 16}), gamedata::global::tileSize.mulComponents(Vector2{40.0f, 32.0f}), *m_application->getRenderer());
    m_registry.get<CameraFocusArea>(newfocus).overrideFocusArea({getTilePos(Vector2{42.5f, 15.0f}), gamedata::global::tileSize.mulComponents(Vector2{7.0f, 30.0f}) / 2.0f});

    newfocus = m_registry.create();
    m_registry.emplace<CameraFocusArea>(newfocus, getTilePos(Vector2{58, 16}), gamedata::global::tileSize.mulComponents(Vector2{40.0f, 32.0f}), *m_application->getRenderer());
    m_registry.get<CameraFocusArea>(newfocus).overrideFocusArea({getTilePos(Vector2{58.0f, 15.0f}), gamedata::global::tileSize.mulComponents(Vector2{24.0f, 30.0f}) / 2.0f});

    newfocus = m_registry.create();
    m_registry.emplace<CameraFocusArea>(newfocus, getTilePos(Vector2{76, 16}), gamedata::global::tileSize.mulComponents(Vector2{40.0f, 32.0f}), *m_application->getRenderer());
    m_registry.get<CameraFocusArea>(newfocus).overrideFocusArea({getTilePos(Vector2{80.25f, 15.0f}), gamedata::global::tileSize.mulComponents(Vector2{20.5f, 30.0f}) / 2.0f});
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

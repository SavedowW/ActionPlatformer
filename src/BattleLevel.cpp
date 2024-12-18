#include "BattleLevel.h"
#include "TileMapHelper.hpp"
#include "World.h"
#include "Profile.h"

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
    m_hudsys(m_registry, *application_, m_camera, lvlId_, size_, m_lastFullFrameTime),
    m_enemysys(m_registry, *application_, m_navsys, m_camera, m_partsys),
    m_aisys(m_registry),
    m_navsys(m_registry, *application_, m_graph),
    m_colsys(m_registry),
    m_graph(*application_),
    m_partsys(m_registry, *application_),
    m_battlesys(m_registry, *application_, m_camera)
{
    auto playerId = m_registry.create();
    m_registry.emplace<ComponentTransform>(playerId, Vector2{313.34f, 352.0f}, ORIENTATION::RIGHT);
    m_registry.emplace<ComponentPhysical>(playerId);
    m_registry.emplace<ComponentObstacleFallthrough>(playerId);
    m_registry.emplace<ComponentAnimationRenderable>(playerId);
    m_registry.emplace<RenderLayer<1>>(playerId);
    m_registry.emplace<ComponentPlayerInput>(playerId, std::unique_ptr<InputResolver>(new InputResolver(application_->getInputSystem())));
    m_registry.emplace<ComponentDynamicCameraTarget>(playerId);
    m_registry.emplace<World>(playerId, m_registry, m_camera, m_partsys, m_navsys);
    m_registry.emplace<StateMachine>(playerId);
    m_registry.emplace<Navigatable>(playerId);
    m_registry.emplace<PhysicalEvents>(playerId);
    m_registry.emplace<BattleActor>(playerId, BattleTeams::PLAYER);

    m_playerId = playerId;
    m_camsys.m_playerId = playerId;
    m_hudsys.m_playerId = playerId;
    m_enemysys.m_playerId = playerId;

    m_enemysys.makeEnemy();

    m_tlmap.load("Tiles/tiles");

    LevelBuilder bld(*application_, m_registry);
    m_decor = std::move(bld.buildLevel("Resources/Sprites/Tiles/tilemap.json", m_tlmap, playerId, m_graph));

    /*auto newcld = m_registry.create();
    m_registry.emplace<ComponentTransform>(newcld, getTilePos(Vector2{20.0f, 21.0f}), ORIENTATION::RIGHT);
    m_registry.emplace<ComponentStaticCollider>(newcld, getTilePos(Vector2{20.0f, 21.0f}), SlopeCollider({0.0f, 0.0f}, Vector2{16.0f, 16.0f}, 1), 0);
    m_registry.emplace<MoveCollider2Points>(newcld, getTilePos(Vector2{20.0f, 17.0f}), getTilePos(Vector2{15.0f, 21.0f}), 180.0f);

    newcld = m_registry.create();
    m_registry.emplace<ComponentTransform>(newcld, getTilePos(Vector2{17.0f, 21.0f}), ORIENTATION::RIGHT);
    m_registry.emplace<ComponentStaticCollider>(newcld, getTilePos(Vector2{17.0f, 21.0f}), SlopeCollider({0.0f, 0.0f}, Vector2{16.0f, 0.0f}, -1), 0);
    m_registry.emplace<MoveCollider2Points>(newcld, getTilePos(Vector2{17.0f, 17.5f}), getTilePos(Vector2{12.0f, 21.5f}), 180.0f);

    newcld = m_registry.create();
    m_registry.emplace<ComponentTransform>(newcld, getTilePos(Vector2{19.0f, 21.0f}), ORIENTATION::RIGHT);
    m_registry.emplace<ComponentStaticCollider>(newcld, getTilePos(Vector2{19.0f, 21.0f}), SlopeCollider({0.0f, 0.0f}, Vector2{16.0f * 2, 16.0f}, 0), 0);
    m_registry.emplace<MoveCollider2Points>(newcld, getTilePos(Vector2{18.5f, 17.0f}), getTilePos(Vector2{13.5f, 21.0f}), 180.0f);*/
}

void BattleLevel::enter()
{
    Level::enter();

    m_playerSystem.setup(m_playerId);

    m_camera.setScale(gamedata::global::minCameraScale);
    m_camera.setPos({0.0f, 0.0f});
}

void BattleLevel::update()
{
    PROFILE_FUNCTION;

    /*
        ComponentPhysical - read / write
    */
    m_physsys.prepHitstop();

    /*
        ComponentPlayerInput - read and write
    */
    m_inputsys.update();

    /*
        ComponentAnimationRenderable - read and write
    */
    m_rendersys.update();

    /*
        ComponentAI, ComponentTransform (own) - read and write
        Player's transform, physics, possibly state machine - read
    */
    m_aisys.update();

    /*
        StateMachine - read / write
        StateMachine represents physical state of a character and can potentially access almost everyting related to all entites
    */
    m_physsys.updateSMs();

    /*
        Physics - write
    */
    m_physsys.prepEntities();

    /*
        ComponentStaticCollider - read and write
        SwitchCollider - read and write
    */
    m_colsys.updateSwitchingColliders();

    /*
        ComponentStaticCollider - read and write
        MoveCollider2Points - read and write
    */
    m_colsys.updateMovingColliders();

    /*
        ComponentParticlePrimitive - read / write
    */
    m_partsys.update();

    /*
        ComponentStaticCollider - read
        Transform, physics, fallthrough, PhysicalEvents, ComponentParticlePhysics - read / write
    */
    m_physsys.updatePhysics();
    
    /*
        ComponentStaticCollider, transform, physics - read
        fallthrough - read / write
    */
    m_physsys.updateOverlappedObstacles();

    /*
        StateMachine, PhysicalEvents - read / write
    */
    m_physsys.updatePhysicalEvents();

    /*
        ComponentTransform, StateMachine - read
        BattleActor - read / write
    */
   m_battlesys.update();
   m_battlesys.handleAttacks();

    /*
        ComponentDynamicCameraTarget (player) - read / write
        ComponentTransform, ComponentPhysical (player) - read
        CameraFocusArea - read
    */
    m_camsys.update();

    /*
        ComponentTransform - read
        Navigatable - read / write
    */
    m_navsys.update();

    /*
        Just updates camera shake logic, but many systems can cause shake
    */
    m_camera.update();
}

void BattleLevel::draw()
{
    PROFILE_FUNCTION;

    auto &renderer = *m_application->getRenderer();
    renderer.prepareRenderer(gamedata::colors::LVL2);

    m_decor.draw(m_camera);

    m_rendersys.draw();

    m_graph.draw(m_camera);
    m_navsys.draw(m_camera);

    m_camsys.debugDraw(renderer, m_camera);

    m_battlesys.debugDraw();

    m_hudsys.draw();

    renderer.updateScreen(m_camera);
}

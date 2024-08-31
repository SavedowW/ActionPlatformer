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
    m_partsys(m_registry, *application_)
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

    auto nd1 = m_graph.makeNode(Vector2{0, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd2 = m_graph.makeNode(Vector2{6, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd3 = m_graph.makeNode(Vector2{10, 21}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd4 = m_graph.makeNode(Vector2{21, 21}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd5 = m_graph.makeNode(Vector2{25, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd6 = m_graph.makeNode(Vector2{22, 27}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd7 = m_graph.makeNode(Vector2{21, 27}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd8 = m_graph.makeNode(Vector2{19, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd9 = m_graph.makeNode(Vector2{10, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});

    auto nd10 = m_graph.makeNode(Vector2{29, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd11 = m_graph.makeNode(Vector2{33, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd12 = m_graph.makeNode(Vector2{35, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd13 = m_graph.makeNode(Vector2{36, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd14 = m_graph.makeNode(Vector2{37, 25}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd15 = m_graph.makeNode(Vector2{39, 22}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd16 = m_graph.makeNode(Vector2{40, 22}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd17 = m_graph.makeNode(Vector2{40, 19}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd18 = m_graph.makeNode(Vector2{41, 19}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd19 = m_graph.makeNode(Vector2{42, 19}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd20 = m_graph.makeNode(Vector2{47, 19}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});

    auto nd21 = m_graph.makeNode(Vector2{38, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd22 = m_graph.makeNode(Vector2{49, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});
    auto nd23 = m_graph.makeNode(Vector2{53, 29}.mulComponents(gamedata::global::tileSize) + Vector2{0.0f, gamedata::global::tileSize.y / 2.0f});

    m_graph.makeConnection(nd1, nd2, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd2, nd3, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP),
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd3, nd4, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd4, nd5, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd2, nd9, 
        Traverse::makeSignature(true, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd8, nd9, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd7, nd8, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd6, nd7, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP),
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd5, nd6, 
        Traverse::makeSignature(true, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd5, nd10, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd10, nd11, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd11, nd12, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd12, nd13, 
        Traverse::makeSignature(false, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::FALL));

    m_graph.makeConnection(nd13, nd14, 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd14, nd21,
        Traverse::makeSignature(false, TraverseTraits::FALL), 
        Traverse::makeSignature(false, TraverseTraits::JUMP));

    m_graph.makeConnection(nd21, nd22,
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd22, nd23,
        Traverse::makeSignature(false, TraverseTraits::WALK), 
        Traverse::makeSignature(false, TraverseTraits::WALK));

    m_graph.makeConnection(nd14, nd15,
        Traverse::makeSignature(false, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::JUMP));

    m_graph.makeConnection(nd15, nd16,
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd15, nd17,
        Traverse::makeSignature(false, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::FALL));

    m_graph.makeConnection(nd17, nd18,
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd18, nd19,
        Traverse::makeSignature(false, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::JUMP));

    m_graph.makeConnection(nd19, nd20,
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP), 
        Traverse::makeSignature(false, TraverseTraits::WALK, TraverseTraits::JUMP));

    m_graph.makeConnection(nd20, nd22,
        Traverse::makeSignature(false, TraverseTraits::FALL), 
        Traverse::makeSignature(false));

    m_playerId = playerId;
    m_camsys.m_playerId = playerId;
    m_hudsys.m_playerId = playerId;
    m_enemysys.m_playerId = playerId;

    //m_enemysys.makeEnemy();

    m_tlmap.load("Tiles/tiles");

    LevelBuilder bld(*application_, m_registry);
    m_decor = std::move(bld.buildLevel("Resources/Sprites/Tiles/tilemap.json", m_tlmap, playerId));

    auto newcld = m_registry.create();
    m_registry.emplace<ComponentStaticCollider>(newcld, SlopeCollider(getTilePos(Vector2{20.0f, 21.0f}), Vector2{16.0f, 16.0f}, 1), 0);
    m_registry.emplace<MoveCollider2Points>(newcld, getTilePos(Vector2{20.0f, 17.0f}), getTilePos(Vector2{15.0f, 21.0f}), 180.0f);

    newcld = m_registry.create();
    m_registry.emplace<ComponentStaticCollider>(newcld, SlopeCollider(getTilePos(Vector2{17.0f, 21.0f}), Vector2{16.0f, 0.0f}, -1), 0);
    m_registry.emplace<MoveCollider2Points>(newcld, getTilePos(Vector2{17.0f, 17.5f}), getTilePos(Vector2{12.0f, 21.5f}), 180.0f);

    newcld = m_registry.create();
    m_registry.emplace<ComponentStaticCollider>(newcld, SlopeCollider(getTilePos(Vector2{19.0f, 21.0f}), Vector2{16.0f * 2, 16.0f}, 0), 0);
    m_registry.emplace<MoveCollider2Points>(newcld, getTilePos(Vector2{18.5f, 17.0f}), getTilePos(Vector2{13.5f, 21.0f}), 180.0f);
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
    PROFILE_FUNCTION;

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

    m_hudsys.draw();

    renderer.updateScreen(m_camera);
}

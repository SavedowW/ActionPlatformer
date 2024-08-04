#include "BattleLevel.h"
#include "Application.h"
#include "TileMapHelper.hpp"

void addCollider(entt::registry &reg_, const SlopeCollider &cld_)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentStaticCollider>(newid, cld_);
}

void addTrigger(entt::registry &reg_, const Trigger &trg_)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentTrigger>(newid, trg_);
}

void addCollider(entt::registry &reg_, const SlopeCollider &cld_, int id_)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentStaticCollider>(newid, cld_);
    reg_.emplace<ComponentObstacle>(newid, id_);
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

    addTrigger(m_registry, createTrigger({6, 4}, {6, 10}, Trigger::Tag::LEFT | Trigger::Tag::ClingArea));
    addCollider(m_registry, getColliderForTileRange(Vector2{6, 4}, Vector2{1, 7}, 0));
    addTrigger(m_registry, createTrigger({6, 4}, {6, 10}, Trigger::Tag::RIGHT | Trigger::Tag::ClingArea));

    addCollider(m_registry, getColliderForTileRange(Vector2{1, 19}, Vector2{1, 1}, 0));

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
    renderer.prepareRenderer(SDL_Color{ 31, 24, 51, 255 });

    m_decor.draw(m_camera);

    m_rendersys.draw();

    m_hudsys.draw();

    renderer.updateScreen(m_camera);
}

PlayerSystem::PlayerSystem(entt::registry &reg_, Application &app_) :
    m_reg(reg_),
    m_animManager(*app_.getAnimationManager())
{
    
}

void PlayerSystem::setup()
{
    auto view = m_reg.view<ComponentTransform, ComponentPhysical, ComponentPlayerInput, ComponentAnimationRenderable, StateMachine>();
    view.each([&m_animManager = this->m_animManager](auto idx_, ComponentTransform &trans, ComponentPhysical &phys, ComponentPlayerInput &inp, ComponentAnimationRenderable &animrnd, StateMachine &sm)
    {
        animrnd.m_animations[m_animManager.getAnimID("Char1/idle")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/idle"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/run")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/run"), LOOPMETHOD::JUMP_LOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/prejump")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/prejump"), LOOPMETHOD::JUMP_LOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/float")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/float"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/attack1_1")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/attack1_1"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/attack1_2")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/attack1_2"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/WallCling")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/WallCling"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/FloatCling")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/FloatCling"), LOOPMETHOD::NOLOOP);

        animrnd.m_currentAnimation = animrnd.m_animations[m_animManager.getAnimID("Char1/float")].get();
        animrnd.m_currentAnimation->reset();


        phys.m_pushbox = {Vector2{0.0f, -30.0f}, Vector2{10.0f, 30.0f}};
        phys.m_gravity = {0.0f, 0.5f};


        inp.m_inputResolver->subscribePlayer();
        inp.m_inputResolver->setInputEnabled(true);


        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerState<false, true, InputComparatorTapUpLeft, InputComparatorTapUpRight, true, InputComparatorIdle, InputComparatorIdle>(
                CharacterState::PREJUMP_FORWARD, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::RUN}}, m_animManager.getAnimID("Char1/prejump")))
            ->setAlignedSlopeMax(0.5f)
            .setGroundedOnSwitch(true)
            .setGravity({{0.0f, 0.0f}})
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>(8.0f))
            .setUpdateMovementData(
                TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {0.0f, 0.0f}},
                        {3, {3.5f, 0.0f}},
                    }), // Dir vel mul
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {0.0f, 0.0f}},
                        {3, {0.0f, -5.0f}},
                    }), // Raw vel
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {1.0f, 1.0f}},
                        {3, {0.5f, 1.0f}}
                    }), // Inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
            .setOutdatedTransition(CharacterState::FLOAT, 3)
            .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        ));

        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerState<false, false, InputComparatorTapUp, InputComparatorTapUp, true, InputComparatorIdle, InputComparatorIdle>(
                CharacterState::PREJUMP, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::RUN}}, m_animManager.getAnimID("Char1/prejump")))
            ->setGroundedOnSwitch(true)
            .setGravity({{0.0f, 0.0f}})
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>(8.0f))
            .setUpdateMovementData(
                TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir vel mul
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {0.0f, 0.0f}},
                        {4, {0.0f, -6.0f}},
                    }), // Raw vel
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {1.0f, 1.0f}},
                        {4, {0.5f, 1.0f}}
                    }), // Inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
            .setOutdatedTransition(CharacterState::FLOAT, 4)
            .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .setUpdateSpeedLimitData(
                TimelineProperty<Vector2<float>>(),
                TimelineProperty<Vector2<float>>({9999.9f, 4.0f}))
        ));

        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerState<false, true, InputComparatorHoldLeft, InputComparatorHoldRight, true, InputComparatorHoldLeft, InputComparatorHoldRight>(
                CharacterState::RUN, {CharacterState::NONE, {CharacterState::IDLE}}, m_animManager.getAnimID("Char1/run")))
            ->setGroundedOnSwitch(true)
            .setGravity({{0.0f, 0.0f}})
            .setUpdateMovementData(
                TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
                TimelineProperty<Vector2<float>>( // Dir vel mul
                    {
                        {0, {0.3f, 0.0f}},
                        {5, {0.6f, 0.0f}},
                    }),  
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Raw vel
                TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
            .setUpdateSpeedLimitData(
                TimelineProperty<Vector2<float>>({3.75f, 0.0f}),
                TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>(10.0f))
            .setCanFallThrough(TimelineProperty(true))
        ));

        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerState<false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>(
                CharacterState::IDLE, {CharacterState::NONE, {CharacterState::RUN}}, m_animManager.getAnimID("Char1/idle")))
            ->setGroundedOnSwitch(true)
            .setGravity({{0.0f, 0.0f}})
            .setDrag(TimelineProperty<Vector2<float>>({{0, Vector2{0.1f, 0.1f}}, {3, Vector2{0.5f, 0.5f}}}))
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>(8.0f))
            .setCanFallThrough(TimelineProperty(true))
            .setUpdateSpeedLimitData(
                TimelineProperty<Vector2<float>>({9999.9f, 0.0f}),
                TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
        ));

        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerActionFloat(
                m_animManager.getAnimID("Char1/float"), {CharacterState::NONE, {}}))
            ->setGroundedOnSwitch(false)
            .setTransitionOnTouchedGround(CharacterState::IDLE)
            .setGravity({{0.0f, 0.5f}})
            .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .setNoLanding(TimelineProperty<bool>({{0, true}, {2, false}}))
        ));

        sm.setInitialState(CharacterState::FLOAT);
    });
}

void PlayerSystem::update()
{
    auto view = m_reg.view<ComponentTransform, StateMachine, ComponentDynamicCameraTarget>();
    auto *ptr = &m_reg;
    for (auto [entity, trans, sm, dtar] : view.each())
    {
        sm.update({ptr, entity}, 0);
        std::cout << sm << std::endl;
    }
}

RenderSystem::RenderSystem(entt::registry &reg_, Application &app_, Camera &camera_) :
    m_reg(reg_),
    m_renderer(*app_.getRenderer()),
    m_camera(camera_)
{
}

void RenderSystem::draw()
{
    auto viewColliders = m_reg.view<ComponentStaticCollider>();
    auto viewTriggers = m_reg.view<ComponentTrigger>();
    auto viewInstances = m_reg.view<ComponentTransform, ComponentAnimationRenderable>();
    auto viewPhysical = m_reg.view<ComponentTransform, ComponentPhysical>();
    auto viewFocuses = m_reg.view<CameraFocusArea>();

    for (auto [idx, scld] : viewColliders.each())
    {
        if (m_reg.all_of<ComponentObstacle>(idx))
            drawObstacle(scld);
        else
            drawCollider(scld);
    }

    for (auto [idx, trg] : viewTriggers.each())
        drawTrigger(trg);

    for (auto [idx, trans, inst] : viewInstances.each())
        drawInstance(trans, inst);

    for (auto [idx, trans, phys] : viewPhysical.each())
        drawCollider(trans, phys);

    for (auto [idx, area] : viewFocuses.each())
        drawFocusArea(area);
}

void RenderSystem::drawInstance(ComponentTransform &trans_, ComponentAnimationRenderable &ren_)
{
    if (ren_.m_currentAnimation != nullptr)
    {
        auto texSize = ren_.m_currentAnimation->getSize();
        auto animorigin = ren_.m_currentAnimation->getOrigin();
        auto texPos = trans_.m_pos;
        texPos.y -= animorigin.y;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (trans_.m_orientation == ORIENTATION::LEFT)
        {
            flip = SDL_FLIP_HORIZONTAL;
            texPos.x -= (texSize.x - animorigin.x);
        }
        else
        {
            texPos.x -= animorigin.x;
        }

        auto spr = ren_.m_currentAnimation->getSprite();
        auto edge = ren_.m_currentAnimation->getBorderSprite();

        m_renderer.renderTexture(spr, texPos.x, texPos.y, texSize.x , texSize.y, m_camera, 0.0f, flip);
    }
}

void RenderSystem::drawCollider(ComponentTransform &trans_, ComponentPhysical &phys_)
{
    if (gamedata::debug::drawColliders)
    {
        auto pb = phys_.m_pushbox + trans_.m_pos;;
        m_renderer.drawCollider(pb, {238, 195, 154, 50}, 100, m_camera);
    }
}

void RenderSystem::drawCollider(ComponentStaticCollider &cld_)
{
    if (gamedata::debug::drawColliders)
    {
        m_renderer.drawCollider(cld_.m_collider, {255, 0, 0, 100}, 255, m_camera);
    }
}

void RenderSystem::drawObstacle(ComponentStaticCollider &cld_)
{
    if (gamedata::debug::drawColliders)
    {
        m_renderer.drawCollider(cld_.m_collider, {50, 50, 255, 100}, 255, m_camera);
    }
}

void RenderSystem::drawTrigger(ComponentTrigger &cld_)
{
    if (gamedata::debug::drawColliders)
    {
        m_renderer.drawCollider(cld_.m_trigger, {255, 50, 255, 50}, 100, m_camera);
    }
}

void RenderSystem::drawFocusArea(CameraFocusArea &cfa_)
{
    cfa_.draw(m_camera);
}

InputHandlingSystem::InputHandlingSystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void InputHandlingSystem::update()
{
    auto view = m_reg.view<ComponentPlayerInput>();
    for (auto [idx, inp] : view.each())
    {
        inp.m_inputResolver->update();
    };
}

PhysicsSystem::PhysicsSystem(entt::registry &reg_, Vector2<float> levelSize_) :
    m_reg(reg_),
    m_levelSize(levelSize_)
{
}

void PhysicsSystem::update()
{
    //m_physicalQuery{reg_.makeQuery<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough>()},
    //m_staticColliderQuery{reg_.makeQuery<ComponentStaticCollider>()},
    auto viewPhys = m_reg.view<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough>();
    auto viewscld = m_reg.view<ComponentStaticCollider>();

    for (auto [idx, trans, phys, obsfall] : viewPhys.each())
    {
        if (m_reg.all_of<StateMachine>(idx))
            proceedEntity(viewscld, idx, trans, phys, obsfall, &m_reg.get<StateMachine>(idx));
        else
            proceedEntity(viewscld, idx, trans, phys, obsfall, nullptr);
    }
}

void PhysicsSystem::proceedEntity(auto &clds_, entt::entity idx_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, StateMachine *sm_)
{
    // Common stuff
    phys_.m_velocity += phys_.m_gravity;

    if (phys_.m_inertia.x != 0)
    {
        auto absInertia = abs(phys_.m_inertia.x);
        auto m_inertiaSign = phys_.m_inertia.x / abs(phys_.m_inertia.x);
        absInertia = std::max(absInertia - phys_.m_drag.x, 0.0f);
        phys_.m_inertia.x = m_inertiaSign * absInertia;
    }

    if (phys_.m_inertia.y != 0)
    {
        auto absInertia = abs(phys_.m_inertia.y);
        auto m_inertiaSign = phys_.m_inertia.y / abs(phys_.m_inertia.y);
        absInertia = std::max(absInertia - phys_.m_drag.y, 0.0f);
        phys_.m_inertia.y = m_inertiaSign * absInertia;
    }

    //std::cout << m_staticColliderQuery.size() << std::endl;

    // Prepare vars for collision detection
    auto offset = phys_.getPosOffest();
    auto pb = phys_.m_pushbox + trans_.m_pos;
    bool noUpwardLanding = phys_.m_noUpwardLanding;

    auto oldHeight = trans_.m_pos.y;
    auto oldTop = pb.getTopEdge();
    auto oldRightEdge = pb.getRightEdge();
    auto oldLeftEdge = pb.getLeftEdge();

    bool groundCollision = false;
    float touchedSlope = 0.0f;
    float highest = m_levelSize.y;

    // Fall collision detection - single collider vs single entity
    auto resolveFall = [&](const ComponentStaticCollider &csc_, int obstacleId_)
    {
        if (oldTop >= csc_.m_collider.m_points[2].y)
            return;
        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        if ((overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y))
        {
            if (obstacleId_ && (!obsFallthrough_.touchedObstacleTop(obstacleId_) || highest < oldHeight))
                return;

            //std::cout << "Touched slope top, teleporting on top, offset.y > 0\n";

            trans_.m_pos.y = highest;
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            groundCollision = true;
            pb = phys_.m_pushbox + trans_.m_pos;

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
    };

    // Rise collision detection
    auto resolveRise = [&](const ComponentStaticCollider &csc_, int obstacleId_)
    {
        if (csc_.m_collider.m_highestSlopePoint > oldTop)
            return;

        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        if ((overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y))
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleBottom(obstacleId_))
                return;

            std::cout << "Touched ceiling, teleporting to bottom, offset.y < 0\n";

            auto overlapPortion = utils::getOverlapPortion(pb.getLeftEdge(), pb.getRightEdge(), csc_.m_collider.m_points[0].x, csc_.m_collider.m_points[1].x);

            if (overlapPortion >= 0.7f)
            {
                phys_.m_velocity.y = 0;
                phys_.m_inertia.y = 0;
            }

            trans_.m_pos.y = csc_.m_collider.m_points[2].y + pb.getSize().y;
            pb = phys_.m_pushbox + trans_.m_pos;
        }
    };
    
    // Movement to right collision detection
    auto resolveRight = [&](const ComponentStaticCollider &csc_, int obstacleId_)
    {
        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        bool aligned = csc_.m_collider.getOrientationDir() > 0;
        auto heightDiff = abs(csc_.m_collider.m_lowestSlopePoint - pb.getBottomEdge());

        if ((overlap & utils::OverlapResult::TOUCH_MIN1_MAX2) && (overlap & utils::OverlapResult::BOTH_OOT))
        {
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            groundCollision = true;
        }

        if (aligned && pb.getTopEdge() <= csc_.m_collider.m_lowestSlopePoint && (overlap & utils::OverlapResult::OOT_SLOPE) && (overlap & utils::OverlapResult::OVERLAP_Y)) // Touched slope from right direction
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleSlope(obstacleId_))
                return;

            std::cout << "Touched slope, teleporting on top, offset.x > 0\n";

            trans_.m_pos.y = highest;
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            pb = phys_.m_pushbox + trans_.m_pos;
            groundCollision = true;

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
        else if ((!aligned || csc_.m_collider.m_hasBox) && (overlap & utils::OverlapResult::OOT_BOX) && (overlap & utils::OverlapResult::OVERLAP_Y) && pb.getBottomEdge() > csc_.m_collider.m_lowestSlopePoint) // Touched inner box
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleSide(obstacleId_))
                return;

            auto overlapPortion = utils::getOverlapPortion(pb.getTopEdge(), pb.getBottomEdge(), highest, csc_.m_collider.m_points[2].y);

            if (overlapPortion >= 0.1 || !phys_.m_onSlopeWithAngle != 0)
            {
                std::cout << "Touched edge, teleporting to it, offset.x > 0\n";
                trans_.m_pos.x = csc_.m_collider.m_tlPos.x - pb.m_halfSize.x;
                pb = phys_.m_pushbox + trans_.m_pos;
            }

            if (overlapPortion >= 0.15)
            {
                std::cout << "Hard collision, limiting speed\n";
                if (phys_.m_velocity.x > 0)
                    phys_.m_velocity.x = 0;
                if (phys_.m_inertia.x > 0)
                    phys_.m_inertia.x = 0;
            }
        }
    };

    // Movement to left collision detection
    auto resolveLeft = [&](const ComponentStaticCollider &csc_, int obstacleId_)
    {
        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        bool aligned = csc_.m_collider.getOrientationDir() < 0;
        auto heightDiff = abs(csc_.m_collider.m_lowestSlopePoint - pb.getBottomEdge());

        if ((overlap & utils::OverlapResult::TOUCH_MIN1_MAX2) && (overlap & utils::OverlapResult::BOTH_OOT))
        {
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            groundCollision = true;
        }

        if (aligned && pb.getTopEdge() <= csc_.m_collider.m_lowestSlopePoint && (overlap & utils::OverlapResult::OOT_SLOPE) && (overlap & utils::OverlapResult::OVERLAP_Y)) // Touched slope from right direction
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleSlope(obstacleId_))
                return;

            std::cout << "Touched slope, teleporting on top, offset.x < 0\n";

            trans_.m_pos.y = highest;
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            pb = phys_.m_pushbox + trans_.m_pos;
            groundCollision = true;

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
        else if ((!aligned || csc_.m_collider.m_hasBox) && (overlap & utils::OverlapResult::OOT_BOX) && (overlap & utils::OverlapResult::OVERLAP_Y) && pb.getBottomEdge() > csc_.m_collider.m_lowestSlopePoint) // Touched inner box
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleSide(obstacleId_))
                return;

            auto overlapPortion = utils::getOverlapPortion(pb.getTopEdge(), pb.getBottomEdge(), highest, csc_.m_collider.m_points[2].y);

            if (overlapPortion >= 0.1 || !phys_.m_onSlopeWithAngle != 0)
            {
                std::cout << "Touched edge, teleporting to it, offset.x < 0\n";
                trans_.m_pos.x = csc_.m_collider.m_tlPos.x + csc_.m_collider.m_size.x + pb.m_halfSize.x;
                pb = phys_.m_pushbox + trans_.m_pos;
            }

            if (overlapPortion >= 0.15)
            {
                std::cout << "Hard collision, limiting speed\n";
                if (phys_.m_velocity.x < 0)
                    phys_.m_velocity.x = 0;
                if (phys_.m_inertia.x < 0)
                    phys_.m_inertia.x = 0;
            }
        }
    };

    // Iteration over colliders depending on archetype
    auto distrbObstacle = [&clds_, &reg = this->m_reg](auto idx, ComponentStaticCollider& cld_, auto &distrib_)
    {
        if (reg.all_of<ComponentObstacle>(idx))
            distrib_(cld_, reg.get<ComponentObstacle>(idx).m_obstacleId);
        else
            distrib_(cld_, 0);
    };

    // X axis movement handling
    {
        trans_.m_pos.x += offset.x;
        pb = phys_.m_pushbox + trans_.m_pos;

        // Moving to the right

        if (offset.x > 0)
        {
            for (auto [idx, cld] : clds_.each())
                distrbObstacle(idx, cld, resolveRight);
        }
        // Moving to the left
        else if (offset.x < 0)
        {
            for (auto [idx, cld] : clds_.each())
                distrbObstacle(idx, cld, resolveLeft);
        }
    }

    // Y axis movement handling
    {
        trans_.m_pos.y += offset.y;
        pb = phys_.m_pushbox + trans_.m_pos;

        // Falling / staying in place
        if (offset.y >= 0)
        {
            for (auto [idx, cld] : clds_.each())
                distrbObstacle(idx, cld, resolveFall);
        }
        // Rising
        else
        {
            if (noUpwardLanding)
            {
                touchedSlope = 0;
                groundCollision = false;
            }

            for (auto [idx, cld] : clds_.each())
                distrbObstacle(idx, cld, resolveRise);
        }
    }

    resetEntityObstacles(trans_, phys_, obsFallthrough_);
    phys_.m_onSlopeWithAngle = touchedSlope;

    if (sm_)
    {
        auto *currentState = sm_->getRealCurrentState();

        if (groundCollision)
        {
            currentState->onTouchedGround({&m_reg, idx_});
        }
        else
        {
            if (!magnetEntity(clds_, trans_, phys_, obsFallthrough_))
                currentState->onLostGround({&m_reg, idx_});
        }
    }

    obsFallthrough_.m_isIgnoringObstacles.update();
}

bool PhysicsSystem::magnetEntity(auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_)
{
    if (phys_.m_magnetLimit <= 0.0f)
        return false;

    auto pb = phys_.m_pushbox + trans_.m_pos;

    float height = trans_.m_pos.y;
    if ( getHighestVerticalMagnetCoord(clds_, pb, height, obsFallthrough_.m_ignoredObstacles, obsFallthrough_.m_isIgnoringObstacles.isActive()))
    {
        float magnetRange = height - trans_.m_pos.y;
        if (magnetRange <= phys_.m_magnetLimit)
        {
            std::cout << "MAGNET: " << magnetRange << std::endl;
            trans_.m_pos.y = height;
            return true;
        }
    }

    return false;
}

bool PhysicsSystem::getHighestVerticalMagnetCoord(auto &clds_, const Collider &cld_, float &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_)
{
    float baseCoord = coord_;
    float bot = cld_.getBottomEdge();
    bool isFound = false;
    auto proceedCollider = [&ignoreAllObstacles_, &ignoredObstacles_, &baseCoord, &isFound, &coord_, &cld_](const SlopeCollider &areaCld_, int obstacleId_)
    {
        if (obstacleId_ && (ignoreAllObstacles_ || ignoredObstacles_.contains(obstacleId_)))
            return;

        auto horOverlap = utils::getOverlap<0>(areaCld_.m_points[0].x, areaCld_.m_points[1].x, cld_.getLeftEdge(), cld_.getRightEdge()); //cld.getHorizontalOverlap(cld_);
        if (!!(horOverlap & utils::OverlapResult::OVERLAP_X))
        {
            auto height = areaCld_.getTopHeight(cld_, horOverlap);
            if (height > baseCoord && (!isFound || height < coord_))
            {
                coord_ = height;
                isFound = true;
            }
        }
    };

    // Fall iteration over colliders depending on archetype
    auto distrbObstacle = [&proceedCollider, &reg = this->m_reg](auto idx, ComponentStaticCollider& cld_)
    {
        if (reg.all_of<ComponentObstacle>(idx))
        {
            proceedCollider(cld_.m_collider, reg.get<ComponentObstacle>(idx).m_obstacleId);
        }
        else
        {
            proceedCollider(cld_.m_collider, 0);
        }
    };

    clds_.each(distrbObstacle);

    return isFound;
}

void PhysicsSystem::resetEntityObstacles(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_)
{
    auto touched = getTouchedObstacles(phys_.m_pushbox + trans_.m_pos);
    std::set<int> res;
    std::set_intersection(
        obsFallthrough_.m_ignoredObstacles.begin(), obsFallthrough_.m_ignoredObstacles.end(),
        touched.begin(), touched.end(),
        std::inserter(res, res.begin()));

    if (obsFallthrough_.m_ignoredObstacles.size() != res.size())
        std::cout << "Reseted obstacles\n";

    obsFallthrough_.m_ignoredObstacles = res;
}

std::set<int> PhysicsSystem::getTouchedObstacles(const Collider &pb_)
{
    auto viewObstacles = m_reg.view<ComponentStaticCollider, ComponentObstacle>();
    std::set<int> obstacleIds;
    float dumped = 0.0f;

    auto proceedObstacle = [&](const SlopeCollider &areaCld_, int obstacleId_) 
    {
        if (obstacleIds.contains(obstacleId_))
            return false;

        return !!(areaCld_.getFullCollisionWith(pb_, dumped) & utils::OverlapResult::BOTH_OOT);
    };

    for (auto [idx, cld, obst] : viewObstacles.each())
    {
        if (proceedObstacle(cld.m_collider, obst.m_obstacleId))
            obstacleIds.insert(obst.m_obstacleId);
    }

    return obstacleIds;
}

CameraSystem::CameraSystem(entt::registry &reg_, Camera &cam_) :
    m_reg(reg_),
    m_cam(cam_)
{
}

void CameraSystem::update()
{
    Vector2<float> target;
    auto [trans, phys, dtar] = m_reg.get<ComponentTransform, ComponentPhysical, ComponentDynamicCameraTarget>(m_playerId);
    if ((phys.m_velocity + phys.m_inertia) != Vector2{0.0f, 0.0f})
    {
        auto tarcamoffset = utils::limitVectorLength((phys.m_velocity + phys.m_inertia).mulComponents(Vector2{1.0f, 0.0f}) * 30, 100.0f);
        auto deltaVec = tarcamoffset - dtar.m_offset;
        auto dlen = deltaVec.getLen();
        auto ddir = deltaVec.normalised();
        float offsetLen = pow(dlen, 2.0f) / 400.0f;
        offsetLen = utils::clamp(offsetLen, 0.0f, dlen);

        dtar.m_offset += ddir * offsetLen;
    }

    target = trans.m_pos - Vector2{0.0f, 60.0f} + dtar.m_offset;

    if (updateFocus(phys.m_pushbox + trans.m_pos))
    {
        auto &area = m_reg.get<CameraFocusArea>(*m_currentFocusArea);
        m_cam.smoothMoveTowards(area.getCameraTargetPosition(target), {1.0f, 1.0f}, 0, 1.3f, 20.0f);
        m_cam.smoothScaleTowards(area.getScale());
    }
    else
    {
        m_cam.smoothMoveTowards(target, {1.0f, 0.5f}, 5.0f, 1.6f, 80.0f);
        m_cam.smoothScaleTowards(gamedata::global::maxCameraScale);
    }
}

bool CameraSystem::updateFocus(const Collider &playerPb_)
{
    if (m_currentFocusArea)
    {
        auto &area = m_reg.get<CameraFocusArea>(*m_currentFocusArea);
        if (area.checkIfEnters(playerPb_, true))
            return true;
        else
            m_currentFocusArea.reset();
    }

    auto areas = m_reg.view<CameraFocusArea>();

    for (auto [idx, area]: areas.each())
    {
        if (area.checkIfEnters(playerPb_, false))
        {
            m_currentFocusArea = idx;
            return true;
        }
    }

    return false;
}

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

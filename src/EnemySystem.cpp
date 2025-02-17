#include "EnemySystem.h"
#include "CoreComponents.h"
#include "StateMachine.h"
#include "Enemy1.h"
#include "NavGraph.h"

EnemySystem::EnemySystem(entt::registry &reg_, Application &app_, NavSystem &navsys_, Camera &cam_, ParticleSystem &partsys_) :
    m_reg(reg_),
    m_animManager(*app_.getAnimationManager()),
    m_navsys(navsys_),
    m_cam(cam_),
    m_partsys(partsys_)
{
}

void EnemySystem::makeEnemy()
{
    auto enemyId = m_reg.create();
    m_reg.emplace<ComponentTransform>(enemyId, Vector2{170.0f, 300.0f}, ORIENTATION::RIGHT);
    m_reg.emplace<PhysicalEvents>(enemyId);
    m_reg.emplace<BattleActor>(enemyId, BattleTeams::ENEMIES);

    auto &nav = m_reg.emplace<Navigatable>(enemyId);
    nav.m_validTraitsOwnLocation = Traverse::makeSignature(true, TraverseTraits::WALK, TraverseTraits::JUMP,  TraverseTraits::FALL);
    nav.m_traverseTraits = Traverse::makeSignature(true, TraverseTraits::WALK, TraverseTraits::JUMP,  TraverseTraits::FALL);
    nav.m_currentOwnConnection = nullptr;
    nav.m_maxRange = 60.0f;
    nav.m_checkIfGrounded = true;

    m_reg.emplace<World>(enemyId, m_reg, m_cam, m_partsys, m_navsys);
    m_reg.emplace<ComponentObstacleFallthrough>(enemyId);


    auto &ai = m_reg.emplace<ComponentAI>(enemyId);
    ai.m_requestedState = static_cast<CharState>(Enemy1State::IDLE);
    ai.m_requestedOrientation = ORIENTATION::RIGHT;
    ai.m_navigationTarget = {430, 410};

    m_reg.emplace<HealthOwner>(enemyId, 3);
    m_reg.emplace<HealthRendererCommonWRT>(enemyId, 3, m_animManager, Vector2{0.0f, -28.0f});

    /*auto *proxySwitchState = new ProxySelectionState(
        Enemy1State::META_PROXY_SWITCH, Enemy1StateNames.at(Enemy1State::META_PROXY_SWITCH), {Enemy1State::NONE, {}}, 
        {Enemy1State::META_BLIND_CHASE, Enemy1State::META_MOVE_TOWARDS}, {100.0f}, {&m_reg, m_playerId});

    auto *moveTowardsState = new MoveTowards(
        Enemy1State::META_MOVE_TOWARDS, Enemy1StateNames.at(Enemy1State::META_MOVE_TOWARDS), {Enemy1State::NONE, {}}, 
        Enemy1State::RUN);

    proxySwitchState->addState(std::unique_ptr<GenericState>(std::move(moveTowardsState)));

    proxySwitchState->addState(std::unique_ptr<GenericState>(
        new BlindChaseState(
            Enemy1State::META_BLIND_CHASE, Enemy1StateNames.at(Enemy1State::META_BLIND_CHASE), {Enemy1State::NONE, {}},
            Enemy1State::IDLE, Enemy1State::RUN, {&m_reg, m_playerId}, 20.0f)
    ));

    proxySwitchState->setInitialState(Enemy1State::META_ROAM);

    ai.m_sm.addState(std::unique_ptr<GenericState>(std::move(proxySwitchState)));*/

    auto *navigateChase = new NavigateGraphChase(
        Enemy1State::META_NAVIGATE_GRAPH_CHASE, Enemy1StateNames.at(Enemy1State::META_NAVIGATE_GRAPH_CHASE), {Enemy1State::NONE, {}},
        Enemy1State::META_MOVE_TOWARDS, Enemy1State::PREJUMP, Enemy1State::IDLE, Enemy1State::META_BLIND_CHASE);

    navigateChase->addState(std::unique_ptr<GenericState>(
        new MoveTowards(
        Enemy1State::META_MOVE_TOWARDS, Enemy1StateNames.at(Enemy1State::META_MOVE_TOWARDS), {Enemy1State::NONE, {}}, 
        Enemy1State::RUN)
    ));

    navigateChase->addState(std::unique_ptr<GenericState>(
        new JumpTowards(
        Enemy1State::PREJUMP, Enemy1StateNames.at(Enemy1State::PREJUMP), {Enemy1State::NONE, {}},
        Enemy1State::PREJUMP)
    ));

    navigateChase->addState(std::unique_ptr<GenericState>(
        new AIStateNull(
        Enemy1State::IDLE, Enemy1StateNames.at(Enemy1State::IDLE), {Enemy1State::NONE, {}})
    ));

    navigateChase->addState(std::unique_ptr<GenericState>(
        new BlindChaseState(
            Enemy1State::META_BLIND_CHASE, Enemy1StateNames.at(Enemy1State::META_BLIND_CHASE), {Enemy1State::NONE, {}},
            Enemy1State::IDLE, Enemy1State::RUN, 200.0f)
    ));

    navigateChase->setInitialState(Enemy1State::IDLE);
    navigateChase->switchCurrentState({&m_reg, enemyId}, Enemy1State::IDLE);

    ai.m_sm.addState(std::unique_ptr<GenericState>(std::move(navigateChase)));
    ai.m_sm.setInitialState(Enemy1State::META_MOVE_TOWARDS);
    ai.m_sm.switchCurrentState({&m_reg, enemyId}, Enemy1State::META_MOVE_TOWARDS);


    auto &phys = m_reg.emplace<ComponentPhysical>(enemyId);
    phys.m_pushbox = Collider(Vector2{-15, -30}, Vector2{30, 30});
    phys.m_gravity = {0.0f, 0.2f};


    auto &animrnd = m_reg.emplace<ComponentAnimationRenderable>(enemyId);
    m_reg.emplace<RenderLayer>(enemyId, 3);

    animrnd.m_animations[m_animManager.getAnimID("Enemy1/idle")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Enemy1/idle"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[m_animManager.getAnimID("Enemy1/float")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Enemy1/float"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[m_animManager.getAnimID("Enemy1/run")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Enemy1/run"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[m_animManager.getAnimID("Enemy1/prejump")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Enemy1/prejump"), LOOPMETHOD::JUMP_LOOP);

    animrnd.m_currentAnimation = animrnd.m_animations[m_animManager.getAnimID("Enemy1/idle")].get();
    animrnd.m_currentAnimation->reset();

    

    auto &sm = m_reg.emplace<StateMachine>(enemyId);

    sm.addState(std::unique_ptr<GenericState>(
        &(new NPCState<true, true>(
            Enemy1State::RUN, Enemy1StateNames.at(Enemy1State::RUN), {Enemy1State::NONE, {Enemy1State::IDLE, Enemy1State::RUN}}, m_animManager.getAnimID("Enemy1/run")))
        ->setGravity({{0.0f, 0.0f}})
        .setUpdateMovementData(
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
            TimelineProperty<Vector2<float>>( 
                {
                    {0, {0.2f, 0.0f}},
                    {5, {0.4f, 0.0f}},
                }),  // Dir vel mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Raw vel
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty<Vector2<float>>({2.0f, 0.0f}),
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
        .setTransitionOnLostGround(Enemy1State::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(10.0f))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setCanFallThrough(TimelineProperty(true))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new AimedPrejump(
            Enemy1State::PREJUMP, Enemy1StateNames.at(Enemy1State::PREJUMP), {Enemy1State::NONE, {Enemy1State::IDLE, Enemy1State::RUN}}, m_animManager.getAnimID("Enemy1/prejump"),
            0.3f, 1.5f))
        ->setGravity({{0.0f, 0.0f}})
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(Enemy1State::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(4.0f))
        .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        .setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setOutdatedTransition(Enemy1State::FLOAT, 1)
        .setParticlesSingle(TimelineProperty<ParticleTemplate>({
            {0, {}},
            {1, ParticleTemplate{1, Vector2<float>{0.0f, 0.0f}, m_animManager.getAnimID("Char1/particles/particle_jump"), 22,
                4}},
            {2, {}},
            }))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new NPCState<false, false>(
            Enemy1State::IDLE, Enemy1StateNames.at(Enemy1State::IDLE), {Enemy1State::NONE, {Enemy1State::IDLE, Enemy1State::RUN}}, m_animManager.getAnimID("Enemy1/idle")))
        ->setGravity({{0.0f, 0.0f}})
        .setDrag(TimelineProperty<Vector2<float>>({{0, Vector2{0.1f, 0.1f}}, {3, Vector2{0.5f, 0.5f}}}))
        .setConvertVelocityOnSwitch(true, false)
        .setTransitionOnLostGround(Enemy1State::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(8.0f))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setCanFallThrough(TimelineProperty(true))
        .setUpdateSpeedLimitData(
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}),
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
        .setHitStateMapping(std::move(HitStateMapping()
            .addHitstunTransition(0, std::numeric_limits<CharState>::max())
            .addHitstunTransition(2, static_cast<CharState>(Enemy1State::PREJUMP))
            ))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new AimedFloat(
            Enemy1State::FLOAT, Enemy1StateNames.at(Enemy1State::FLOAT), {Enemy1State::NONE, {}}, m_animManager.getAnimID("Enemy1/float")))
        ->setGravity({{0.0f, 0.3f}})
        .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        .setHurtboxes({
            {
                HurtboxGroup(
                    {
                        {
                            {{{-6, -28}, {12, 28}}, TimelineProperty<bool>(true)}
                        }
                    }, HurtTrait::NORMAL
                )
            }
        })
        .setTransitionOnTouchedGround(Enemy1State::IDLE)
        .setNoLanding(TimelineProperty<bool>({{0, true}, {4, false}}))
    ));

    sm.setInitialState(Enemy1State::FLOAT);
}

void EnemySystem::update()
{
    
}

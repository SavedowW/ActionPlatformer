#include "EnemySystem.h"
#include "CoreComponents.h"
#include "StateMachine.h"
#include "Enemy1.h"

EnemySystem::EnemySystem(entt::registry &reg_, Application &app_) :
    m_reg(reg_),
    m_animManager(*app_.getAnimationManager())
{
}

void EnemySystem::makeEnemy()
{
    auto enemyId = m_reg.create();
    m_reg.emplace<ComponentTransform>(enemyId, Vector2{170.0f, 300.0f}, ORIENTATION::RIGHT);
    m_reg.emplace<ComponentObstacleFallthrough>(enemyId);


    auto &ai = m_reg.emplace<ComponentAI>(enemyId);
    ai.m_requestedState = static_cast<CharState>(Enemy1State::IDLE);
    ai.m_requestedOrientation = ORIENTATION::RIGHT;

    auto *proxySwitchState = new ProxySelectionState(
        Enemy1State::META_PROXY_SWITCH, Enemy1StateNames.at(Enemy1State::META_PROXY_SWITCH), {Enemy1State::NONE, {}}, 
        {Enemy1State::META_BLIND_CHASE, Enemy1State::META_ROAM}, {100.0f}, {&m_reg, m_playerId});

    auto *roamState = new RandomRoamState(
        Enemy1State::META_ROAM, Enemy1StateNames.at(Enemy1State::META_ROAM), {Enemy1State::NONE, {}}, 
        Enemy1State::IDLE, Enemy1State::RUN, {50, 180}, {10, 60});

    roamState->addState(std::unique_ptr<GenericState>(
        &(new AIState(
            Enemy1State::IDLE, Enemy1StateNames.at(Enemy1State::IDLE), {Enemy1State::NONE, {}}))
        ->setEnterRequestedState(static_cast<CharState>(Enemy1State::IDLE))
    ));

    roamState->addState(std::unique_ptr<GenericState>(
        &(new AIState(
            Enemy1State::RUN, Enemy1StateNames.at(Enemy1State::RUN), {Enemy1State::NONE, {}}))
        ->setEnterRequestedState(static_cast<CharState>(Enemy1State::RUN))
    ));

    roamState->setInitialState(Enemy1State::IDLE);

    proxySwitchState->addState(std::unique_ptr<GenericState>(std::move(roamState)));

    proxySwitchState->addState(std::unique_ptr<GenericState>(
        new BlindChaseState(
            Enemy1State::META_BLIND_CHASE, Enemy1StateNames.at(Enemy1State::META_BLIND_CHASE), {Enemy1State::NONE, {}},
            Enemy1State::IDLE, Enemy1State::RUN, {&m_reg, m_playerId}, 20.0f)
    ));

    proxySwitchState->setInitialState(Enemy1State::META_ROAM);

    ai.m_sm.addState(std::unique_ptr<GenericState>(std::move(proxySwitchState)));

    ai.m_sm.setInitialState(Enemy1State::META_PROXY_SWITCH);
    ai.m_sm.switchCurrentState({&m_reg, enemyId}, Enemy1State::META_PROXY_SWITCH);


    auto &phys = m_reg.emplace<ComponentPhysical>(enemyId);
    phys.m_pushbox = Collider(Vector2{0.0f, -15.0f}, Vector2{15.0f, 15.0f});
    phys.m_gravity = {0.0f, 0.2f};


    auto &animrnd = m_reg.emplace<ComponentAnimationRenderable>(enemyId);

    animrnd.m_animations[m_animManager.getAnimID("Enemy1/idle")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Enemy1/idle"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[m_animManager.getAnimID("Enemy1/float")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Enemy1/float"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[m_animManager.getAnimID("Enemy1/run")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Enemy1/run"), LOOPMETHOD::JUMP_LOOP);

    animrnd.m_currentAnimation = animrnd.m_animations[m_animManager.getAnimID("Enemy1/idle")].get();
    animrnd.m_currentAnimation->reset();

    

    auto &sm = m_reg.emplace<StateMachine>(enemyId);

    sm.addState(std::unique_ptr<GenericState>(
        &(new NPCState<true, true>(
            Enemy1State::RUN, Enemy1StateNames.at(Enemy1State::RUN), {Enemy1State::NONE, {Enemy1State::IDLE, Enemy1State::RUN}}, m_animManager.getAnimID("Enemy1/run")))
        ->setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
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
        .setCanFallThrough(TimelineProperty(true))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new NPCState<false, false>(
            Enemy1State::IDLE, Enemy1StateNames.at(Enemy1State::IDLE), {Enemy1State::NONE, {Enemy1State::IDLE, Enemy1State::RUN}}, m_animManager.getAnimID("Enemy1/idle")))
        ->setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
        .setDrag(TimelineProperty<Vector2<float>>({{0, Vector2{0.1f, 0.1f}}, {3, Vector2{0.5f, 0.5f}}}))
        .setConvertVelocityOnSwitch(true)
        .setTransitionOnLostGround(Enemy1State::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(8.0f))
        .setCanFallThrough(TimelineProperty(true))
        .setUpdateSpeedLimitData(
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}),
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new NPCState<false, false>(
            Enemy1State::FLOAT, Enemy1StateNames.at(Enemy1State::FLOAT), {Enemy1State::NONE, {Enemy1State::FLOAT}}, m_animManager.getAnimID("Enemy1/float")))
        ->setGroundedOnSwitch(false)
        .setGravity({{0.0f, 0.3f}})
        .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        .setTransitionOnTouchedGround(Enemy1State::IDLE)
        .setNoLanding(TimelineProperty<bool>({{0, true}, {4, false}}))
    ));

    sm.setInitialState(Enemy1State::FLOAT);
}

void EnemySystem::update()
{
    
}

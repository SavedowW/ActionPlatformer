#include "PlayerSystem.h"

PlayerSystem::PlayerSystem(entt::registry &reg_, Application &app_) :
    m_reg(reg_),
    m_animManager(*app_.getAnimationManager())
{
    
}

void PlayerSystem::setup(entt::entity playerId_)
{
    auto [trans, phys, inp, animrnd, sm, nav] = m_reg.get<ComponentTransform, ComponentPhysical, ComponentPlayerInput, ComponentAnimationRenderable, StateMachine, Navigatable>(playerId_);

    if (m_reg.all_of<ComponentSpawnLocation>(playerId_))
    {
        trans.m_pos = m_reg.get<ComponentSpawnLocation>(playerId_).m_location;
    }

    trans.m_orientation = ORIENTATION::RIGHT;

    animrnd.m_animations[m_animManager.getAnimID("Char1/idle")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/idle"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[m_animManager.getAnimID("Char1/run")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/run"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[m_animManager.getAnimID("Char1/prejump")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/prejump"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[m_animManager.getAnimID("Char1/float")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/float"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[m_animManager.getAnimID("Char1/wall_cling")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/wall_cling"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[m_animManager.getAnimID("Char1/wall_prejump")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/wall_prejump"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[m_animManager.getAnimID("Char1/prerun")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/prerun"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[m_animManager.getAnimID("Char1/run_recovery")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/run_recovery"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[m_animManager.getAnimID("Char1/landing_recovery")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/landing_recovery"), LOOPMETHOD::NOLOOP);

    animrnd.m_currentAnimation = animrnd.m_animations[m_animManager.getAnimID("Char1/float")].get();
    animrnd.m_currentAnimation->reset();


    phys.m_pushbox = {Vector2{0.0f, -16.0f}, Vector2{7.0f, 16.0f}};
    phys.m_gravity = {0.0f, 0.5f};


    inp.m_inputResolver->subscribePlayer();
    inp.m_inputResolver->setInputEnabled(true);

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerActionWallPrejump(
            m_animManager.getAnimID("Char1/wall_prejump"), {CharacterState::NONE, {CharacterState::WALL_CLING}}))
        ->setTransitionOnTouchedGround(CharacterState::IDLE)
        .setDrag(TimelineProperty<Vector2<float>>({1.0f, 0.5f}))
        .setOutdatedTransition(CharacterState::FLOAT, 3)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerActionWallCling(
            m_animManager.getAnimID("Char1/wall_cling"), {CharacterState::NONE, {CharacterState::FLOAT}}))
        ->setDrag(TimelineProperty<Vector2<float>>({
            {0, {1.0f, 0.2f}},
            {1, {1.0f, 0.4f}},
            }))
        .setTransitionOnTouchedGround(CharacterState::IDLE)
    ));


    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, true, InputComparatorTapUpLeft, InputComparatorTapUpRight, true, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::PREJUMP_FORWARD, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY, CharacterState::LANDING_RECOVERY}}, m_animManager.getAnimID("Char1/prejump")))
        ->setAlignedSlopeMax(0.5f)
        .setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
        .setConvertVelocityOnSwitch(true)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(4.0f))
        .setUpdateMovementData(
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
            TimelineProperty<Vector2<float>>(
                {
                    {0, {0.0f, 0.0f}},
                    {1, {1.5f, 0.0f}},
                }), // Dir vel mul
            TimelineProperty<Vector2<float>>(
                {
                    {0, {0.0f, 0.0f}},
                    {1, {0.0f, -4.0f}},
                }), // Raw vel
            TimelineProperty<Vector2<float>>(
                {
                    {0, {1.0f, 1.0f}},
                    {1, {0.5f, 1.0f}}
                }), // Inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
        .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        .setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        .setOutdatedTransition(CharacterState::FLOAT, 1)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, false, InputComparatorTapUp, InputComparatorTapUp, true, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::PREJUMP, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY, CharacterState::LANDING_RECOVERY}}, m_animManager.getAnimID("Char1/prejump")))
        ->setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
        .setConvertVelocityOnSwitch(true)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(4.0f))
        .setUpdateMovementData(
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir vel mul
            TimelineProperty<Vector2<float>>(
                {
                    {0, {0.0f, 0.0f}},
                    {1, {0.0f, -4.5f}},
                }), // Raw vel
            TimelineProperty<Vector2<float>>(
                {
                    {0, {1.0f, 1.0f}},
                    {1, {0.5f, 1.0f}}
                }), // Inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
        .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        .setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        .setUpdateSpeedLimitData(
            TimelineProperty<Vector2<float>>(),
            TimelineProperty<Vector2<float>>({9999.9f, 4.0f}))
        .setOutdatedTransition(CharacterState::FLOAT, 1)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<true, false, InputComparatorFail, InputComparatorFail, true, InputComparatorHoldLeft, InputComparatorHoldRight>(
            CharacterState::RUN, {CharacterState::NONE, {}}, m_animManager.getAnimID("Char1/run")))
        ->setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
        .setUpdateMovementData(
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
            TimelineProperty<Vector2<float>>( 
                {
                    {0, {0.4f, 0.0f}},
                    {5, {0.6f, 0.0f}},
                }),  // Dir vel mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Raw vel
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty<Vector2<float>>({2.5f, 0.0f}),
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(4.0f))
        .setCanFallThrough(TimelineProperty(true))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, true, InputComparatorHoldLeft, InputComparatorHoldRight, true, InputComparatorHoldLeft, InputComparatorHoldRight>(
            CharacterState::PRERUN, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::RUN, CharacterState::LANDING_RECOVERY}}, m_animManager.getAnimID("Char1/prerun")))
        ->setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
        .setConvertVelocityOnSwitch(true)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(4.0f))
        .setUpdateMovementData(
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
            TimelineProperty<Vector2<float>>( 
                {
                    {0, {0.0f, 0.0f}},
                    {3, {0.3f, 0.0f}},
                }),  // Dir vel mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Raw vel
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty<Vector2<float>>({2.5f, 0.0f}),
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
        .setOutdatedTransition(CharacterState::RUN, 5)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<true, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::RUN_RECOVERY, {CharacterState::NONE, {CharacterState::PRERUN, CharacterState::RUN}}, m_animManager.getAnimID("Char1/run_recovery")))
        ->setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
        .setDrag({{0.5f, 0.0f}})
        .setConvertVelocityOnSwitch(true)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(4.0f))
        .setUpdateMovementData(
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}),  // Dir vel mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Raw vel
            TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
            TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
        .setUpdateSpeedLimitData(
            TimelineProperty<Vector2<float>>({2.5f, 0.0f}),
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
        .setOutdatedTransition(CharacterState::IDLE, 9)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::IDLE, {CharacterState::NONE, {}}, m_animManager.getAnimID("Char1/idle")))
        ->setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
        .setDrag(TimelineProperty<Vector2<float>>({{0, Vector2{0.1f, 0.1f}}, {3, Vector2{0.5f, 0.5f}}}))
        .setConvertVelocityOnSwitch(true)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(4.0f))
        .setCanFallThrough(TimelineProperty(true))
        .setUpdateSpeedLimitData(
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}),
            TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<true, false, InputComparatorFail, InputComparatorFail, false, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::LANDING_RECOVERY, {CharacterState::NONE, {}}, m_animManager.getAnimID("Char1/landing_recovery")))
        ->setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
        .setConvertVelocityOnSwitch(true)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(4.0f))
        .setOutdatedTransition(CharacterState::IDLE, 14)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerState<true, false, InputComparatorFail, InputComparatorFail, false, InputComparatorIdle, InputComparatorIdle>(
            CharacterState::HARD_LANDING_RECOVERY, {CharacterState::NONE, {}}, m_animManager.getAnimID("Char1/landing_recovery")))
        ->setGroundedOnSwitch(true)
        .setGravity({{0.0f, 0.0f}})
        .setConvertVelocityOnSwitch(true)
        .setTransitionOnLostGround(CharacterState::FLOAT)
        .setMagnetLimit(TimelineProperty<float>(4.0f))
        .setOutdatedTransition(CharacterState::IDLE, 14)
    ));

    sm.addState(std::unique_ptr<GenericState>(
        &(new PlayerActionFloat(
            m_animManager.getAnimID("Char1/float"), {CharacterState::NONE, {}}))
        ->setGroundedOnSwitch(false)
        .setGravity({{0.0f, 0.5f}})
        .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        .setNoLanding(TimelineProperty<bool>({{0, true}, {4, false}}))
    ));

    sm.setInitialState(CharacterState::FLOAT);

    nav.m_currentOwnConnection = nullptr;
    nav.m_isFallthroughOk = true;
    nav.m_maxRange = 60.0f;
    nav.m_validTraitsOwnLocation = makeTraitList(TraverseTraits::WALK);
}

void PlayerSystem::update()
{
}

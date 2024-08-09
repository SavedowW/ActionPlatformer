#include "PlayerSystem.h"

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
        animrnd.m_animations[m_animManager.getAnimID("Char1/idle")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/idle"), LOOPMETHOD::JUMP_LOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/run")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/run"), LOOPMETHOD::JUMP_LOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/prejump")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/prejump"), LOOPMETHOD::JUMP_LOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/float")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/float"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/attack1_1")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/attack1_1"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/attack1_2")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/attack1_2"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/WallCling")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/WallCling"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/FloatCling")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/FloatCling"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/prerun")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/prerun"), LOOPMETHOD::NOLOOP);
        animrnd.m_animations[m_animManager.getAnimID("Char1/run_recovery")] = std::make_unique<Animation>(m_animManager, m_animManager.getAnimID("Char1/run_recovery"), LOOPMETHOD::NOLOOP);

        animrnd.m_currentAnimation = animrnd.m_animations[m_animManager.getAnimID("Char1/float")].get();
        animrnd.m_currentAnimation->reset();


        phys.m_pushbox = {Vector2{0.0f, -16.0f}, Vector2{10.0f, 16.0f}};
        phys.m_gravity = {0.0f, 0.5f};


        inp.m_inputResolver->subscribePlayer();
        inp.m_inputResolver->setInputEnabled(true);

        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerActionWallPrejump(
                m_animManager.getAnimID("Char1/prejump"), {CharacterState::NONE, {CharacterState::WALL_CLING}}))
            ->setTransitionOnTouchedGround(CharacterState::IDLE)
            .setOutdatedTransition(CharacterState::FLOAT, 3)
            .setDrag(TimelineProperty<Vector2<float>>({1.0f, 0.5f}))
        ));

        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerActionWallCling(
                m_animManager.getAnimID("Char1/prejump"), {CharacterState::NONE, {CharacterState::FLOAT}}))
            ->setDrag(TimelineProperty<Vector2<float>>({
                {0, {1.0f, 0.3f}},
                {1, {1.0f, 0.3f}},
                }))
            .setTransitionOnTouchedGround(CharacterState::IDLE)
        ));


        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerState<false, true, InputComparatorTapUpLeft, InputComparatorTapUpRight, true, InputComparatorIdle, InputComparatorIdle>(
                CharacterState::PREJUMP_FORWARD, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY}}, m_animManager.getAnimID("Char1/prejump")))
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
            .setOutdatedTransition(CharacterState::FLOAT, 1)
            .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
        ));

        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerState<false, false, InputComparatorTapUp, InputComparatorTapUp, true, InputComparatorIdle, InputComparatorIdle>(
                CharacterState::PREJUMP, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::PRERUN, CharacterState::RUN, CharacterState::RUN_RECOVERY}}, m_animManager.getAnimID("Char1/prejump")))
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
                        {1, {0.0f, -4.5f}},
                    }), // Raw vel
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {1.0f, 1.0f}},
                        {1, {0.5f, 1.0f}}
                    }), // Inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
            .setOutdatedTransition(CharacterState::FLOAT, 1)
            .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .setUpdateSpeedLimitData(
                TimelineProperty<Vector2<float>>(),
                TimelineProperty<Vector2<float>>({9999.9f, 4.0f}))
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
            .setMagnetLimit(TimelineProperty<float>(10.0f))
            .setCanFallThrough(TimelineProperty(true))
        ));

        sm.addState(std::unique_ptr<GenericState>(
            &(new PlayerState<false, true, InputComparatorHoldLeft, InputComparatorHoldRight, true, InputComparatorHoldLeft, InputComparatorHoldRight>(
                CharacterState::PRERUN, {CharacterState::NONE, {CharacterState::IDLE, CharacterState::RUN}}, m_animManager.getAnimID("Char1/prerun")))
            ->setGroundedOnSwitch(true)
            .setGravity({{0.0f, 0.0f}})
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>(8.0f))
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
            .setMagnetLimit(TimelineProperty<float>(8.0f))
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
    }
}

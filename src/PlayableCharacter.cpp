#include "PlayableCharacter.h"

PlayableCharacter::PlayableCharacter(Application &application_, const CollisionArea &cldArea_) :
    ActionCharacter(application_, cldArea_),
    m_inputResolver(application_.getInputSystem())
{
    m_inputResolver.subscribePlayer();
    m_inputResolver.setInputEnabled(true);

    AnimationManager animmgmgt = *application_.getAnimationManager();

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new WallClingPrejump<CharacterState, decltype(*this)> (
                CharacterState::WALL_PREJUMP, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/WallCling"), StateMarker{CharacterState::NONE, {CharacterState::WALL_CLING}}, *this, m_inputResolver, getComponent<ComponentTransform>(), getComponent<ComponentPhysical>(), getComponent<ComponentAnimationRenderable>()
            ))
            ->setTransitionOnTouchedGround(CharacterState::IDLE)
            .setOutdatedTransition(CharacterState::FLOAT, 3)
            .setDrag(TimelineProperty<Vector2<float>>({1.0f, 0.5f}))
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new WallClingAction<CharacterState, decltype(*this)> (
                CharacterState::WALL_CLING, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/WallCling"), StateMarker{CharacterState::NONE, {CharacterState::FLOAT}}, *this, m_inputResolver, CharacterState::FLOAT, getComponent<ComponentTransform>(), getComponent<ComponentPhysical>(), getComponent<ComponentAnimationRenderable>()
            ))
            ->setDrag(TimelineProperty<Vector2<float>>({
                {0, {1.0f, 0.3f}},
                {1, {1.0f, 0.3f}},
                }))
            .setTransitionOnTouchedGround(CharacterState::IDLE)
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new PlayerAction<CharacterState, false, false, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail, decltype(*this)> (
                CharacterState::ATTACK1_2, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/attack1_2"), StateMarker{CharacterState::NONE, {}}, *this, m_inputResolver, getComponent<ComponentTransform>(), getComponent<ComponentPhysical>(), getComponent<ComponentAnimationRenderable>()
            ))
            ->setRealignOnSwitch(true)
            .setRecoveryFrames(TimelineProperty<StateMarker>({
                {0, StateMarker{CharacterState::NONE, {}}},
                {20, StateMarker{CharacterState::NONE, {CharacterState::PREJUMP, CharacterState::PREJUMP_FORWARD}}},
                {40, StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::PREJUMP, CharacterState::PREJUMP_FORWARD}}}
            }))
            .setGravity({{0.0f, 0.0f}})
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>({
                {0, 8.0f},
                {8, 30.0f},
                {10, 8.0f},
                }))
            .setUpdateMovementData(
                TimelineProperty<Vector2<float>>({
                        {0, {1.0f, 0.0f}},
                        {1, {1.0f, 1.0f}},
                        {9, {0.0f, 1.0f}},
                        {17, {1.0f, 1.0f}}
                    }), // Vel mul
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {0.5f, 0.0f}},
                        {8, {8.0f, 0.0f}},
                        {9, {0.0f, 0.0f}},
                    }), // Dir vel mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Raw vel
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {1.0f, 0.0f}},
                        {1, {1.0f, 1.0f}}
                    }), // Inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
            .setOutdatedTransition(CharacterState::IDLE, 50)
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new PlayerAction<CharacterState, false, false, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail, decltype(*this)> (
                CharacterState::ATTACK1_1, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/attack1_1"), StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::IDLE}}, *this, m_inputResolver, getComponent<ComponentTransform>(), getComponent<ComponentPhysical>(), getComponent<ComponentAnimationRenderable>()
            ))
            ->setRealignOnSwitch(true)
            .setRecoveryFrames(TimelineProperty<StateMarker>({
                {0, StateMarker{CharacterState::NONE, {}}},
                {20, StateMarker{CharacterState::NONE, {CharacterState::PREJUMP, CharacterState::PREJUMP_FORWARD, CharacterState::ATTACK1_2}}},
                {40, StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::PREJUMP, CharacterState::PREJUMP_FORWARD, CharacterState::ATTACK1_2}}}
            }))
            .setGravity({{0.0f, 0.0f}})
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>({
                {0, 8.0f},
                {5, 25.0f},
                {9, 8.0f},
                }))
            .setUpdateMovementData(
                TimelineProperty<Vector2<float>>({
                        {0, {1.0f, 0.0f}},
                        {1, {1.0f, 1.0f}},
                        {8, {0.0f, 1.0f}},
                        {9, {1.0f, 1.0f}}
                    }), // Vel mul
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {0.5f, 0.0f}},
                        {5, {4.0f, 0.0f}},
                        {8, {0.0f, 0.0f}},
                    }), // Dir vel mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Raw vel
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {1.0f, 0.0f}},
                        {1, {1.0f, 1.0f}}
                    }), // Inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir inr mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f})) // Raw inr
            .setOutdatedTransition(CharacterState::IDLE, 46)
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new PlayerAction<CharacterState, false, true, InputComparatorTapUpLeft, InputComparatorTapUpRight, true, InputComparatorTapUpLeft, InputComparatorTapUpRight, decltype(*this)> (
                CharacterState::PREJUMP_FORWARD, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/prejump"), StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::IDLE}}, *this, m_inputResolver, getComponent<ComponentTransform>(), getComponent<ComponentPhysical>(), getComponent<ComponentAnimationRenderable>()
            ))
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
            .setCooldown(&m_cooldowns[0], 5)
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new PlayerAction<CharacterState, false, false, InputComparatorTapUp, InputComparatorTapUp, true, InputComparatorTapUp, InputComparatorTapUp, decltype(*this)> (
                CharacterState::PREJUMP, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/prejump"), StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::IDLE}}, *this, m_inputResolver, getComponent<ComponentTransform>(), getComponent<ComponentPhysical>(), getComponent<ComponentAnimationRenderable>()
            ))
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
            .setCooldown(&m_cooldowns[0], 5)
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new PlayerAction<CharacterState, false, true, InputComparatorHoldLeft, InputComparatorHoldRight, true, InputComparatorHoldLeft, InputComparatorHoldRight, decltype(*this)> (
                CharacterState::RUN, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/run"), StateMarker{CharacterState::NONE, {CharacterState::IDLE}}, *this, m_inputResolver, getComponent<ComponentTransform>(), getComponent<ComponentPhysical>(), getComponent<ComponentAnimationRenderable>()
            ))
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
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new PlayerAction<CharacterState, false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle, decltype(*this)> (
                CharacterState::IDLE, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/idle"), StateMarker{CharacterState::NONE, {CharacterState::RUN}}, *this, m_inputResolver, getComponent<ComponentTransform>(), getComponent<ComponentPhysical>(), getComponent<ComponentAnimationRenderable>()
            ))
            ->setGroundedOnSwitch(true)
            .setGravity({{0.0f, 0.0f}})
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>(8.0f))
            .setCanFallThrough(TimelineProperty(true))
            .setUpdateSpeedLimitData(
                TimelineProperty<Vector2<float>>({9999.9f, 0.0f}),
                TimelineProperty<Vector2<float>>({9999.9f, 0.0f}))
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new ActionFloat<CharacterState, decltype(*this)> (
                CharacterState::FLOAT, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/float"), StateMarker{CharacterState::NONE, {}}, *this, m_inputResolver, getComponent<ComponentTransform>(), getComponent<ComponentPhysical>(), getComponent<ComponentAnimationRenderable>()
            ))
            ->setGroundedOnSwitch(false)
            .setTransitionOnTouchedGround(CharacterState::IDLE)
            .setGravity({{0.0f, 0.5f}})
            .setDrag(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .addTransitionAnim(CharacterState::WALL_PREJUMP, animmgmgt.getAnimID("Char1/FloatCling"))
        )
    );

    m_currentAction = getAction(CharacterState::FLOAT);
}

void PlayableCharacter::update()
{
    auto &transform = getComponent<ComponentTransform>();
    auto &physical = getComponent<ComponentPhysical>();
    auto &ignoreObstacles = getComponent<ComponentObstacleFallthrough>();

    for (auto &cd : m_cooldowns)
        cd.update();

    m_inputResolver.update();

    if (m_currentAction->canFallThrough(m_framesInState) && m_fallthroughInput(m_inputResolver.getInputQueue(), 0))
        ignoreObstacles.setIgnoringObstacles();

    ActionCharacter::update();

    m_preEditVelocity = physical.m_velocity;

    if ((physical.m_velocity + physical.m_inertia) != Vector2{0.0f, 0.0f})
        {
            auto tarcamoffset = utils::limitVectorLength((physical.m_velocity + physical.m_inertia).mulComponents(Vector2{1.0f, 0.0f}) * 30, 100.0f);
            auto deltaVec = tarcamoffset - m_cameraOffset;
            auto dlen = deltaVec.getLen();
            auto ddir = deltaVec.normalised();
            float offsetLen = pow(dlen, 2.0f) / 400.0f;
            offsetLen = utils::clamp(offsetLen, 0.0f, dlen);

            m_cameraOffset += ddir * offsetLen;
        }
}

void PlayableCharacter::loadAnimations(Application &application_)
{
    AnimationManager animmgmgt = *application_.getAnimationManager();

    auto &animrnd = getComponent<ComponentAnimationRenderable>();

    animrnd.m_animations[animmgmgt.getAnimID("Char1/idle")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/idle"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[animmgmgt.getAnimID("Char1/run")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/run"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[animmgmgt.getAnimID("Char1/prejump")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/prejump"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[animmgmgt.getAnimID("Char1/float")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/float"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[animmgmgt.getAnimID("Char1/attack1_1")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/attack1_1"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[animmgmgt.getAnimID("Char1/attack1_2")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/attack1_2"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[animmgmgt.getAnimID("Char1/WallCling")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/WallCling"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[animmgmgt.getAnimID("Char1/FloatCling")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/FloatCling"), LOOPMETHOD::NOLOOP);

    animrnd.m_currentAnimation = animrnd.m_animations[animmgmgt.getAnimID("Char1/idle")].get();
    animrnd.m_currentAnimation->reset();
}

Vector2<float> PlayableCharacter::getCameraFocusPoint() const
{
    return getComponent<ComponentTransform>().m_pos - Vector2{0.0f, 60.0f} + m_cameraOffset;
}

PlayableCharacter::CharacterGenericAction *PlayableCharacter::getCurrentAction() const
{
    return m_currentAction;
}

const char *PlayableCharacter::getCurrentActionName() const
{
    return CharacterStateNames.at(m_currentAction->m_ownState);
}

Vector2<float> &PlayableCharacter::accessPreEditVelocity()
{
    return m_preEditVelocity;
}

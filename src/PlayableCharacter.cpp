#include "PlayableCharacter.h"

PlayableCharacter::PlayableCharacter(Application &application_, Vector2<float> pos_, const CollisionArea &cldArea_) :
    Object(application_, pos_),
    m_renderer(*application_.getRenderer()),
    m_inputResolver(application_.getInputSystem()),
    m_collisionArea(cldArea_),
    m_onSlopeWithAngle(0)
{
    m_inputResolver.subscribePlayer();
    m_inputResolver.setInputEnabled(true);

    AnimationManager animmgmgt = *application_.getAnimationManager();

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new Action<CharacterState, false, false, InputComparatorTapAttack, InputComparatorTapAttack, false, InputComparatorFail, InputComparatorFail, decltype(*this)> (
                CharacterState::ATTACK_1, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/attack1"), StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::IDLE}}, *this, m_inputResolver
            ))
            ->setGravity({{0.0f, 0.0f}})
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>({
                {0, 8.0f},
                {5, 20.0f},
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
            &(new Action<CharacterState, false, true, InputComparatorTapUpLeft, InputComparatorTapUpRight, true, InputComparatorTapUpLeft, InputComparatorTapUpRight, decltype(*this)> (
                CharacterState::PREJUMP_FORWARD, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/prejump"), StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::IDLE}}, *this, m_inputResolver
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
            .setDrag(TimelineProperty<float>(0.0f))
            .setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .setCooldown(&m_cooldowns[0], 5)
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new Action<CharacterState, false, false, InputComparatorTapUp, InputComparatorTapUp, true, InputComparatorTapUp, InputComparatorTapUp, decltype(*this)> (
                CharacterState::PREJUMP, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/prejump"), StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::IDLE}}, *this, m_inputResolver
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
            .setDrag(TimelineProperty<float>(0.0f))
            .setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>>({0.0f, 0.0f}))
            .setUpdateSpeedLimitData(
                TimelineProperty<Vector2<float>>(),
                TimelineProperty<Vector2<float>>({9999.9f, 4.0f}))
            .setCooldown(&m_cooldowns[0], 5)
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new Action<CharacterState, false, true, InputComparatorHoldLeft, InputComparatorHoldRight, true, InputComparatorHoldLeft, InputComparatorHoldRight, decltype(*this)> (
                CharacterState::RUN, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/run"), StateMarker{CharacterState::NONE, {CharacterState::IDLE}}, *this, m_inputResolver
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
            &(new Action<CharacterState, false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle, decltype(*this)> (
                CharacterState::IDLE, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/idle"), StateMarker{CharacterState::NONE, {CharacterState::RUN}}, *this, m_inputResolver
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
                CharacterState::FLOAT, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/float"), StateMarker{CharacterState::NONE, {}}, *this, m_inputResolver
            ))
            ->setGroundedOnSwitch(false)
            .setTransitionOnTouchedGround(CharacterState::IDLE)
            .setGravity({{0.0f, 0.5f}})
            .setDrag(TimelineProperty<float>(0.0f))
        )
    );

    m_currentAction = getAction(CharacterState::FLOAT);
}

void PlayableCharacter::update()
{
    for (auto &cd : m_cooldowns)
        cd.update();


    m_isIgnoringObstacles.update();

    m_inputResolver.update();
    if (transitionState())
    {
        m_currentAction->onUpdate();
        m_currentAnimation->update();
    }
    std::cout << m_inputResolver.getInputQueue()[0] << std::endl;

    Object::update();

    m_preEditVelocity = m_velocity;

    if ((m_velocity + m_inertia) != Vector2{0.0f, 0.0f})
    {
        auto tarcamoffset = utils::limitVectorLength((m_velocity + m_inertia).mulComponents(Vector2{1.0f, 0.0f}) * 30, 100.0f);
        auto deltaVec = tarcamoffset - m_cameraOffset;
        auto dlen = deltaVec.getLen();
        auto ddir = deltaVec.normalised();
        float offsetLen = pow(dlen, 2.0f) / 400.0f;
        offsetLen = utils::clamp(offsetLen, 0.0f, dlen);

        m_cameraOffset += ddir * offsetLen;
    }
}

void PlayableCharacter::draw(Camera &cam_)
{
    if (m_currentAnimation != nullptr)
    {
        auto texSize = m_currentAnimation->getSize();
        auto animorigin = m_currentAnimation->getOrigin();
        auto texPos = m_pos;
        texPos.y -= animorigin.y;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (m_ownOrientation == ORIENTATION::LEFT)
        {
            flip = SDL_FLIP_HORIZONTAL;
            texPos.x -= (texSize.x - animorigin.x);
        }
        else
        {
            texPos.x -= animorigin.x;
        }

        auto spr = m_currentAnimation->getSprite();
        auto edge = m_currentAnimation->getBorderSprite();

        m_renderer.renderTexture(spr, texPos.x, texPos.y, texSize.x , texSize.y, cam_, 0.0f, flip);
        //m_renderer.renderTexture(edge, texPos.x, texPos.y, texSize.x , texSize.y, cam_, 0.0f, flip);

        if (gamedata::debug::drawColliders)
        {
            m_renderer.drawCollider(getPushbox(), {238, 195, 154, 50}, 100, cam_);
        }
    }
}

bool PlayableCharacter::attemptResetGround()
{
    auto pb = getPushbox();
    float lim = m_currentAction->getMagnetLimit(m_framesInState);
    if (lim <= 0.0f)
        return false;

    float height = m_pos.y;
    if (m_collisionArea.getHighestVerticalMagnetCoord(pb, height, *this))
    {
        float magnetRange = height - m_pos.y;
        if (magnetRange <= lim)
        {
            std::cout << "MAGNET: " << magnetRange << std::endl;
            m_pos.y = height;
            return true;
        }
    }

    return false;
}

bool PlayableCharacter::isIgnoringAllObstacles()
{
    if (m_currentAction->canFallThrough(m_framesInState) && m_fallthroughInput(m_inputResolver.getInputQueue(), 0))
        m_isIgnoringObstacles.begin(5);
    return m_isIgnoringObstacles.isActive();
}

void PlayableCharacter::cleanIgnoredObstacles()
{
    auto touched = m_collisionArea.getPlayerTouchingObstacles(getPushbox());
    std::set<int> res;
    std::set_intersection(
        m_ignoredObstacles.begin(), m_ignoredObstacles.end(),
        touched.begin(), touched.end(),
        std::inserter(res, res.begin()));

    m_ignoredObstacles = res;
}

// true if not ignored
bool PlayableCharacter::touchedObstacleTop(int obstacleId_)
{
    if (m_isIgnoringObstacles.isActive())
    {
        m_ignoredObstacles.insert(obstacleId_);
        return false;
    }

    return !m_ignoredObstacles.contains(obstacleId_);
}

bool PlayableCharacter::touchedObstacleBottom(int obstacleId_)
{
    m_ignoredObstacles.insert(obstacleId_);
    return false;
}

bool PlayableCharacter::touchedObstacleSlope(int obstacleId_)
{
    if (m_isIgnoringObstacles.isActive())
    {
        m_ignoredObstacles.insert(obstacleId_);
        return false;
    }
    return !m_ignoredObstacles.contains(obstacleId_);
}

bool PlayableCharacter::touchedObstacleSide(int obstacleId_)
{
    m_ignoredObstacles.insert(obstacleId_);
    return false;
}

bool PlayableCharacter::checkIgnoringObstacle(int obstacleId_) const
{
    return m_isIgnoringObstacles.isActive() || m_ignoredObstacles.contains(obstacleId_);
}

void PlayableCharacter::setSlopeAngle(float angle_)
{
    m_onSlopeWithAngle = angle_;
}

int PlayableCharacter::getSlopeAngle() const
{
    return m_onSlopeWithAngle;
}

PlayableCharacter::CharacterGenericAction *PlayableCharacter::getAction(CharacterState charState_)
{
    for (auto &el : m_actions)
        if (el->m_ownState == charState_)
            return el.get();

    return nullptr;
}

void PlayableCharacter::loadAnimations(Application &application_)
{
    AnimationManager animmgmgt = *application_.getAnimationManager();

    m_animations[animmgmgt.getAnimID("Char1/idle")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/idle"), LOOPMETHOD::NOLOOP);
    m_animations[animmgmgt.getAnimID("Char1/run")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/run"), LOOPMETHOD::JUMP_LOOP);
    m_animations[animmgmgt.getAnimID("Char1/prejump")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/prejump"), LOOPMETHOD::JUMP_LOOP);
    m_animations[animmgmgt.getAnimID("Char1/float")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/float"), LOOPMETHOD::NOLOOP);
    m_animations[animmgmgt.getAnimID("Char1/attack1")] = std::make_unique<Animation>(animmgmgt, animmgmgt.getAnimID("Char1/attack1"), LOOPMETHOD::NOLOOP);

    m_currentAnimation = m_animations[animmgmgt.getAnimID("Char1/idle")].get();
    m_currentAnimation->reset();
}

Vector2<float> PlayableCharacter::getCurrentGravity() const
{
    auto grav = m_currentAction->getGravity(m_framesInState);
    if (m_velocity.y + m_inertia.y > 0)
        grav *= 1.3f;
    return grav;
}

bool PlayableCharacter::transitionState()
{
    m_framesInState++;
    
    for (auto &el : m_actions)
    {
        bool proceed = false;
        auto res = el->isPossibleInDirection(0, proceed);
        if (proceed)
        {
            m_ownOrientation = res;
            return true;
        }
        if (res != ORIENTATION::UNSPECIFIED)
        {
            m_ownOrientation = res;
            switchTo(el.get());
            return false;
        }
    }

    return true;
}

Collider PlayableCharacter::getPushbox() const
{
    Collider cld = {-10, -60, 20, 60};
    return cld + m_pos;
}

Vector2<float> PlayableCharacter::getCameraFocusPoint() const
{
    return m_pos - Vector2{0.0f, 60.0f} + m_cameraOffset;
}

CharacterState PlayableCharacter::getCurrentActionState() const
{
    return m_currentAction->m_ownState;
}

const char *PlayableCharacter::getCurrentActionName() const
{
    return CharacterStateNames.at(m_currentAction->m_ownState);
}

void PlayableCharacter::switchTo(CharacterState state_)
{
    switchTo(getAction(state_));
}

void PlayableCharacter::switchTo(CharacterGenericAction *charAction_)
{
    m_currentAction = charAction_;
    m_currentAction->onSwitchTo();
}

void PlayableCharacter::onTouchedGround()
{
    if (!isGrounded)
    {
        isGrounded = true;
        m_currentAction->onTouchedGround();

        if (m_inertia.y > 0)
            m_inertia.y = 0;

        if (m_velocity.y > 0)
            m_velocity.y = 0;
    }
}

void PlayableCharacter::onLostGround()
{
    if (isGrounded)
    {
        isGrounded = false;
        m_currentAction->onLostGround();
    }
}

Vector2<float> &PlayableCharacter::accessPreEditVelocity()
{
    return m_preEditVelocity;
}

float PlayableCharacter::getInertiaDrag() const
{
    return m_currentAction->getDrag(m_framesInState);
}

Vector2<float> PlayableCharacter::getInertiaMultiplier() const
{
    return m_currentAction->getAppliedInertiaMultiplier(m_framesInState);
}

#include "PlayableCharacter.h"

PlayableCharacter::PlayableCharacter(Application &application_, Vector2<float> pos_, const CollisionArea &cldArea_) :
    Object(application_, pos_),
    m_renderer(*application_.getRenderer()),
    m_inputResolver(application_.getInputSystem()),
    m_collisionArea(cldArea_)
{
    m_inputResolver.subscribePlayer();
    m_inputResolver.setInputEnabled(true);

    AnimationManager animmgmgt = *application_.getAnimationManager();

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new Action<CharacterState, false, true, InputComparatorTapUpLeft, InputComparatorTapUpRight, true, InputComparatorTapUpLeft, InputComparatorTapUpRight, decltype(*this)> (
                CharacterState::PREJUMP_FORWARD, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/prejump"), StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::IDLE}}, *this, m_inputResolver
            ))
            ->setGravity({{0.0f, 0.0f}})
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
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new Action<CharacterState, false, false, InputComparatorTapUp, InputComparatorTapUp, true, InputComparatorTapUp, InputComparatorTapUp, decltype(*this)> (
                CharacterState::PREJUMP, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/prejump"), StateMarker{CharacterState::NONE, {CharacterState::RUN, CharacterState::IDLE}}, *this, m_inputResolver
            ))
            ->setGravity({{0.0f, 0.0f}})
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>(8.0f))
            .setUpdateMovementData(
                TimelineProperty<Vector2<float>>({1.0f, 1.0f}), // Vel mul
                TimelineProperty<Vector2<float>>({0.0f, 0.0f}), // Dir vel mul
                TimelineProperty<Vector2<float>>(
                    {
                        {0, {0.0f, 0.0f}},
                        {3, {0.0f, -6.0f}},
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
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new Action<CharacterState, false, true, InputComparatorHoldLeft, InputComparatorHoldRight, true, InputComparatorHoldLeft, InputComparatorHoldRight, decltype(*this)> (
                CharacterState::RUN, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/run"), StateMarker{CharacterState::NONE, {CharacterState::IDLE}}, *this, m_inputResolver
            ))
            ->setGravity({{0.0f, 0.0f}})
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
                TimelineProperty<float>(3.75f),
                TimelineProperty<float>())
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>(10.0f))
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new Action<CharacterState, false, false, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle, decltype(*this)> (
                CharacterState::IDLE, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/idle"), StateMarker{CharacterState::NONE, {CharacterState::RUN}}, *this, m_inputResolver
            ))
            ->setGravity({{0.0f, 0.0f}})
            .setConvertVelocityOnSwitch(true)
            .setTransitionOnLostGround(CharacterState::FLOAT)
            .setMagnetLimit(TimelineProperty<float>(8.0f))
        )
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new ActionFloat<CharacterState, decltype(*this)> (
                CharacterState::FLOAT, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/float"), StateMarker{CharacterState::NONE, {}}, *this, m_inputResolver
            ))
            ->setTransitionOnTouchedGround(CharacterState::IDLE)
            .setGravity({{0.0f, 0.5f}})
            .setDrag(TimelineProperty<float>(0.0f))
        )
    );

    m_currentAction = getAction(CharacterState::FLOAT);
}

void PlayableCharacter::update()
{
    m_inputResolver.update();
    if (transitionState())
    {
        m_currentAction->onUpdate();
        m_currentAnimation->update();
    }

    Object::update();

    m_preEditVelocity = m_velocity;
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

        m_renderer.drawCollider(getPushbox(), {238, 195, 154, 50}, 100, cam_);
    }
}

bool PlayableCharacter::attemptResetGround()
{
    auto pb = getPushbox();
    float lim = m_currentAction->getMagnetLimit(m_framesInState);
    if (lim <= 0.0f)
        return false;

    float height = m_pos.y;
    if (m_collisionArea.getHighestVerticalMagnetCoord(pb, height))
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

    m_currentAnimation = m_animations[animmgmgt.getAnimID("Char1/idle")].get();
    m_currentAnimation->reset();
}

Vector2<float> PlayableCharacter::getCurrentGravity() const
{
    return m_currentAction->getGravity(m_framesInState);
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

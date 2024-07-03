#include "PlayableCharacter.h"

PlayableCharacter::PlayableCharacter(Application &application_, Vector2<float> pos_) :
    Object(application_, pos_),
    m_renderer(*application_.getRenderer()),
    InputReactor(application_.getInputSystem()),
    m_inputResolver(application_.getInputSystem())
{
    subscribe(EVENTS::RIGHT);
    subscribe(EVENTS::LEFT);
    setInputEnabled(true);

    AnimationManager animmgmgt = *application_.getAnimationManager();

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            new Action<CharacterState, false, true, InputComparatorHoldLeft, InputComparatorHoldRight, decltype(*this)> (
                CharacterState::RUN, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/run"), StateMarker{CharacterState::NONE, {CharacterState::IDLE}}, *this, m_inputResolver
            ))
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            new Action<CharacterState, false, false, InputComparatorIdle, InputComparatorIdle, decltype(*this)> (
                CharacterState::IDLE, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/idle"), StateMarker{CharacterState::NONE, {CharacterState::RUN}}, *this, m_inputResolver
            ))
    );

    m_actions.push_back(
        std::unique_ptr<CharacterGenericAction>(
            &(new Action<CharacterState, false, true, InputComparatorIdle, InputComparatorIdle, decltype(*this)> (
                CharacterState::FLOAT, Collider{-10, -60, 20, 60}, animmgmgt.getAnimID("Char1/idle"), StateMarker{CharacterState::NONE, {CharacterState::IDLE}}, *this, m_inputResolver
            ))
            ->setTransitionOnTouchedGround(CharacterState::IDLE)
        )
    );

    m_currentAction = getAction(CharacterState::FLOAT);
}

void PlayableCharacter::update()
{
    transitionState();

    m_currentAnimation->update();
    m_velocity += m_gravity;
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

void PlayableCharacter::receiveInput(EVENTS event, const float scale_)
{
    switch (event)
    {
    case EVENTS::RIGHT:
        if (scale_ > 0)
        {
            m_ownOrientation = ORIENTATION::RIGHT;
            m_currentAnimation = m_animations[2].get();
            m_velocity.x = 0.07;
        }
        else
        {
            m_currentAnimation = m_animations[0].get();
            m_velocity.x = 0;
        }
        break;
    case EVENTS::LEFT:
        if (scale_ > 0)
        {
            m_ownOrientation = ORIENTATION::LEFT;
            m_currentAnimation = m_animations[2].get();
            m_velocity.x = -0.07;
        }
        else
        {
            m_currentAnimation = m_animations[0].get();
            m_velocity.x = 0;
        }
        break;
    }
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

    m_currentAnimation = m_animations[animmgmgt.getAnimID("Char1/idle")].get();
    m_currentAnimation->reset();
}

void PlayableCharacter::transitionState()
{
    for (auto &el : m_actions)
    {
        auto res = el->isPossibleInDirection(0);
        if (res != ORIENTATION::UNSPECIFIED)
        {
            m_ownOrientation = res;
            m_currentAction = el.get();
            break;
        }
    }
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
    m_currentAction = getAction(state_);
}

void PlayableCharacter::onTouchedGround()
{
    m_currentAction->onTouchedGround();
}

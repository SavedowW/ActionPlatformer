#include "PlayableCharacter.h"

PlayableCharacter::PlayableCharacter(Application &application_, Vector2<float> pos_) :
    Object(application_, pos_),
    m_pos(pos_),
    m_renderer(*application_.getRenderer()),
    InputReactor(application_.getInputSystem())
{
    subscribe(EVENTS::RIGHT);
    setInputEnabled(true);
}

void PlayableCharacter::update()
{
    m_currentAnimation->update();
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
    }
}

void PlayableCharacter::receiveInput(EVENTS event, const float scale_)
{
    if (scale_ > 0)
        m_currentAnimation = m_animations[2].get();
    else
        m_currentAnimation = m_animations[0].get();
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

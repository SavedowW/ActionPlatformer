#ifndef ACTION_CHARACTER_H_
#define ACTION_CHARACTER_H_
#include "MovableCharacter.h"

template <class OWN_TYPE, typename CHAR_STATE_T>
class ActionCharacter: public MovableCharacter
{
protected:
    using CharacterGenericAction = GenericAction<CHAR_STATE_T, OWN_TYPE&>;

public:
    ActionCharacter(Application &application_, const CollisionArea &cldArea_) :
        MovableCharacter(application_, cldArea_),
        m_renderer(*application_.getRenderer())
    {
    }

    uint32_t getFramesInState() const
    {
        return m_framesInState;
    }

    void switchTo(CHAR_STATE_T state_)
    {
        switchTo(getAction(state_));
    }

    void switchTo(CharacterGenericAction *charAction_)
    {
        charAction_->onSwitchTo();
    }

    virtual void onTouchedGround() override
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

    virtual void onLostGround() override
    {
        if (isGrounded)
        {
            isGrounded = false;
            m_currentAction->onLostGround();
        }
    }

    virtual bool attemptResetGround() override
    {
        auto pb = getPushbox();
        float lim = m_currentAction->getMagnetLimit(m_framesInState);
        if (lim <= 0.0f)
            return false;

        float height = m_pos.y;
        if (m_collisionArea.getHighestVerticalMagnetCoord(pb, height, m_ignoredObstacles))
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

    Collider getPushbox() const
    {
        return m_currentAction->getHurtbox() + m_pos;
    }

    Vector2<float> getInertiaDrag() const
    {
        return m_currentAction->getDrag(m_framesInState);
    }

    Vector2<float> getInertiaMultiplier() const
    {
        return m_currentAction->getAppliedInertiaMultiplier(m_framesInState);
    }

    CHAR_STATE_T getCurrentActionState() const
    {
        return m_currentAction->m_ownState;
    }

    void update()
    {
        if (transitionState())
        {
            m_currentAction->onUpdate();
            m_currentAnimation->update();
        }

        Object::update();
    }

    virtual ~ActionCharacter() = default;

protected:
    CharacterGenericAction *getAction(CHAR_STATE_T charState_)
    {
        for (auto &el : m_actions)
            if (el->m_ownState == charState_)
                return el.get();

        return nullptr;
    }

    Vector2<float> getCurrentGravity() const
    {
        auto grav = m_currentAction->getGravity(m_framesInState);
        if (m_velocity.y + m_inertia.y > 0)
            grav *= 1.3f;
        return grav;
    }

    bool transitionState()
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

    void draw(Camera &cam_)
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
    
            if (gamedata::debug::drawColliders)
            {
                m_renderer.drawCollider(getPushbox(), {238, 195, 154, 50}, 100, cam_);
            }
        }
    }

    std::vector<std::unique_ptr<CharacterGenericAction>> m_actions;
    CharacterGenericAction *m_currentAction;
    uint32_t m_framesInState = 0;
    bool isGrounded = false;

    Renderer &m_renderer;
    std::map<int, std::unique_ptr<Animation>> m_animations;
    Animation *m_currentAnimation;

};

#endif
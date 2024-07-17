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

    virtual void setOnLevel(Application &application_, Vector2<float> pos_) override
    {
        loadAnimations(application_);
        getComponent<ComponentTransform>().m_pos = pos_;
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
        auto &physcomp = getComponent<ComponentPhysical>();
        if (!isGrounded)
        {
            isGrounded = true;
            m_currentAction->onTouchedGround();

            if (physcomp.m_inertia.y > 0)
                physcomp.m_inertia.y = 0;

            if (physcomp.m_velocity.y > 0)
                physcomp.m_velocity.y = 0;
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

        auto &physcomp = getComponent<ComponentPhysical>();

        physcomp.m_gravity = m_currentAction->getGravity(m_framesInState);
        if (physcomp.m_velocity.y + physcomp.m_inertia.y > 0)
            physcomp.m_gravity *= 1.3f;

        physcomp.m_drag = m_currentAction->getDrag(m_framesInState);

        physcomp.m_inertiaMultiplier = m_currentAction->getAppliedInertiaMultiplier(m_framesInState);

        physcomp.m_magnetLimit = m_currentAction->getMagnetLimit(m_framesInState);

        MovableCharacter::update();
    }

    virtual ~ActionCharacter() = default;

protected:
    virtual void loadAnimations(Application &application_) = 0;

    CharacterGenericAction *getAction(CHAR_STATE_T charState_)
    {
        for (auto &el : m_actions)
            if (el->m_ownState == charState_)
                return el.get();

        return nullptr;
    }

    bool transitionState()
    {
        auto &transform = getComponent<ComponentTransform>();
        m_framesInState++;

        for (auto &el : m_actions)
        {
            bool proceed = false;
            auto res = el->isPossibleInDirection(0, proceed);
            if (proceed)
            {
                transform.m_ownOrientation = res;
                return true;
            }
            if (res != ORIENTATION::UNSPECIFIED)
            {
                transform.m_ownOrientation = res;
                switchTo(el.get());
                return false;
            }
        }

        return true;
    }

    void draw(Camera &cam_)
    {
        auto &transform = getComponent<ComponentTransform>();
        if (m_currentAnimation != nullptr)
        {
            auto texSize = m_currentAnimation->getSize();
            auto animorigin = m_currentAnimation->getOrigin();
            auto texPos = transform.m_pos;
            texPos.y -= animorigin.y;
            SDL_RendererFlip flip = SDL_FLIP_NONE;
            if (transform.m_ownOrientation == ORIENTATION::LEFT)
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
                m_renderer.drawCollider(getComponent<ComponentPhysical>().getPushbox(), {238, 195, 154, 50}, 100, cam_);
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
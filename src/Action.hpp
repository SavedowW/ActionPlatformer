#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "Object.h"
#include "InputResolver.h"
#include "InputComparators.h"

/*
    Attempt provided input considering alignment rules
    if REQUIRE_ALIGNMENT is true, only input in specified direction is attempted
    if FORCE_REALIGN is true and real input direction conflicts with specified, input is still valid
    Input in specified direction is ALWAYS prioritized
    currentOrientation_ is expected to be LEFT or RIGHT, not UNSPECIFIED
    Return value is valid direction of input, or UNSPECIFIED if none is valid
*/
template<bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN>
inline ORIENTATION attemptInput(const InputComparator &cmpL_, const InputComparator &cmpR_, ORIENTATION currentOrientation_, const InputQueue &iq_, int val_)
{
    if (REQUIRE_ALIGNMENT)
    {
        if (currentOrientation_ == ORIENTATION::RIGHT && cmpR_(iq_, val_) || currentOrientation_ == ORIENTATION::LEFT && cmpL_(iq_, val_))
            return currentOrientation_;
    }
    else
    {
        auto lValid = cmpL_(iq_, val_);
        auto rValid = cmpR_(iq_, val_);

        if (rValid && currentOrientation_ == ORIENTATION::RIGHT)
            return ORIENTATION::RIGHT;

        if (lValid && currentOrientation_ == ORIENTATION::LEFT)
            return ORIENTATION::LEFT;

        if (!FORCE_REALIGN)
            return ORIENTATION::UNSPECIFIED;

        if (rValid)
            return ORIENTATION::RIGHT;
        else if (lValid)
            return ORIENTATION::LEFT;
    }

    return ORIENTATION::UNSPECIFIED;
}

template<typename CHAR_STATES_T, typename OWNER_T>
class GenericAction
{
public:
    GenericAction(CHAR_STATES_T ownState_, OWNER_T &owner_, int anim_) :
        m_ownState(ownState_),
        m_owner(owner_),
        m_anim(anim_),
        m_drag(1),
        m_appliedInertiaMultiplier({1.0f, 1.0f})
    {
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setTransitionOnTouchedGround(CHAR_STATES_T state_)
    {
        m_transitionOnLand = state_;
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setTransitionOnLostGround(CHAR_STATES_T state_)
    {
        m_transitionOnLostGround = state_;
        return *this;
    }
    
    inline GenericAction<CHAR_STATES_T, OWNER_T> &setGravity(TimelineProperty<Vector2<float>> &&gravity_)
    {
        m_gravity = std::move(gravity_);
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setDrag(TimelineProperty<float> &&drag_)
    {
        m_drag = std::move(drag_);
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setCanFallThrough(TimelineProperty<bool> &&fallThrough_)
    {
        m_canFallThrough = std::move(fallThrough_);
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>> &&inerMul_)
    {
        m_appliedInertiaMultiplier = std::move(inerMul_);
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setConvertVelocityOnSwitch(bool convertVelocity_)
    {
        m_convertVelocityOnSwitch = convertVelocity_;
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setOutdatedTransition(CHAR_STATES_T state_, uint32_t duration_)
    {
        m_transitionOnOutdated = state_;
        m_duration = duration_;
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setUpdateMovementData(
        TimelineProperty<Vector2<float>> &&mulOwnVelUpd_, TimelineProperty<Vector2<float>> &&mulOwnDirVelUpd_, TimelineProperty<Vector2<float>> &&rawAddVelUpd_,
        TimelineProperty<Vector2<float>> &&mulOwnInrUpd_, TimelineProperty<Vector2<float>> &&mulOwnDirInrUpd_, TimelineProperty<Vector2<float>> &&rawAddInrUpd_)
    {
        m_mulOwnVelUpd = std::move(mulOwnVelUpd_);
        m_mulOwnDirVelUpd = std::move(mulOwnDirVelUpd_);
        m_rawAddVelUpd = std::move(rawAddVelUpd_);
        m_mulOwnInrUpd = std::move(mulOwnInrUpd_);
        m_mulOwnDirInrUpd = std::move(mulOwnDirInrUpd_);
        m_rawAddInrUpd = std::move(rawAddInrUpd_);

        m_usingUpdateMovement = !m_mulOwnVelUpd.isEmpty() || !m_mulOwnDirVelUpd.isEmpty() || !m_rawAddVelUpd.isEmpty()
                            || !m_mulOwnInrUpd.isEmpty() || !m_mulOwnDirInrUpd.isEmpty() || !m_rawAddInrUpd.isEmpty();

        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setMagnetLimit(TimelineProperty<float> &&magnetLimit_)
    {
        m_magnetLimit = std::move(magnetLimit_);
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setUpdateSpeedLimitData(TimelineProperty<float> &&ownVelLimitUpd_, TimelineProperty<float> &&ownInrLimitUpd_)
    {
        m_ownVelLimitUpd = std::move(ownVelLimitUpd_);
        m_ownInrLimitUpd = std::move(ownInrLimitUpd_);

        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setGroundedOnSwitch(bool isGrounded_)
    {
        m_setGroundedOnSwitch = isGrounded_;
        return *this;
    }

    inline virtual void onSwitchTo()
    {
        m_owner.m_currentAnimation = m_owner.m_animations[m_anim].get();
        m_owner.m_currentAnimation->reset();
        m_owner.m_currentAction = this;
        m_owner.m_framesInState = 0;

        if (m_convertVelocityOnSwitch)
            m_owner.velocityToInertia();

        if (m_setGroundedOnSwitch.isSet())
            m_owner.isGrounded = m_setGroundedOnSwitch;
    }

    inline virtual void onUpdate()
    {
        if (m_usingUpdateMovement)
        {
            m_owner.m_velocity = m_owner.m_velocity.mulComponents(m_mulOwnVelUpd[m_owner.m_framesInState]) + m_owner.getOwnHorDir().mulComponents(m_mulOwnDirVelUpd[m_owner.m_framesInState]) + m_rawAddVelUpd[m_owner.m_framesInState];
            m_owner.m_inertia = m_owner.m_inertia.mulComponents(m_mulOwnInrUpd[m_owner.m_framesInState]) + m_owner.getOwnHorDir().mulComponents(m_mulOwnDirInrUpd[m_owner.m_framesInState]) + m_rawAddInrUpd[m_owner.m_framesInState];
        }

        if (!m_ownVelLimitUpd.isEmpty())
            m_owner.m_velocity = utils::limitVectorLength(m_owner.m_velocity, m_ownVelLimitUpd[m_owner.m_framesInState]);

        if (!m_ownInrLimitUpd.isEmpty())
            m_owner.m_inertia = utils::limitVectorLength(m_owner.m_inertia, m_ownInrLimitUpd[m_owner.m_framesInState]);

        if (m_transitionOnOutdated.isSet())
        {
            if (m_owner.m_framesInState >= m_duration)
                m_owner.switchTo(m_transitionOnOutdated);
        }
    }

    inline virtual void onTouchedGround()
    {
        if (m_transitionOnLand.isSet())
        {
            m_owner.switchTo(m_transitionOnLand);
        }
    }

    inline virtual void onLostGround()
    {
        if (m_transitionOnLostGround.isSet())
        {
            m_owner.switchTo(m_transitionOnLostGround);
        }
    }

    virtual Vector2<float> getGravity(uint32_t currentFrame_) const
    {
        return m_gravity[currentFrame_];
    }

    virtual float getMagnetLimit(uint32_t currentFrame_) const
    {
        return m_magnetLimit[currentFrame_];
    }

    virtual float getDrag(uint32_t currentFrame_) const
    {
        return m_drag[currentFrame_];
    }

    virtual bool canFallThrough(uint32_t currentFrame_) const
    {
        return m_canFallThrough[currentFrame_];
    }

    virtual Vector2<float> getAppliedInertiaMultiplier(uint32_t currentFrame_) const
    {
        return m_appliedInertiaMultiplier[currentFrame_];
    }

    inline virtual ORIENTATION isPossibleInDirection(int extendBuffer_, bool &isProceed_) const = 0;

    const CHAR_STATES_T m_ownState;

protected:
    OWNER_T &m_owner;
    int m_anim;
    utils::OptionalProperty<CHAR_STATES_T> m_transitionOnLand;
    utils::OptionalProperty<CHAR_STATES_T> m_transitionOnLostGround;
    TimelineProperty<Vector2<float>> m_gravity;

    bool m_usingUpdateMovement = false;
    TimelineProperty<Vector2<float>> m_mulOwnVelUpd;
    TimelineProperty<Vector2<float>> m_mulOwnDirVelUpd;
    TimelineProperty<Vector2<float>> m_rawAddVelUpd;
    TimelineProperty<Vector2<float>> m_mulOwnInrUpd;
    TimelineProperty<Vector2<float>> m_mulOwnDirInrUpd;
    TimelineProperty<Vector2<float>> m_rawAddInrUpd;

    TimelineProperty<Vector2<float>> m_appliedInertiaMultiplier;

    TimelineProperty<float> m_drag;

    bool m_convertVelocityOnSwitch = false;

    TimelineProperty<float> m_ownVelLimitUpd;
    TimelineProperty<float> m_ownInrLimitUpd;

    TimelineProperty<float> m_magnetLimit;

    utils::OptionalProperty<CHAR_STATES_T> m_transitionOnOutdated;
    utils::OptionalProperty<uint32_t> m_duration;

    TimelineProperty<bool> m_canFallThrough;
    utils::OptionalProperty<bool> m_setGroundedOnSwitch;
};

template<typename CHAR_STATES_T, bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN,
    typename CMP_LEFT, typename CMP_RIGHT,
    bool ATTEMPT_PROCEED, typename CMP_PROCEED_LEFT, typename CMP_PROCEED_RIGHT,
    typename OWNER_T>
class Action : public GenericAction<CHAR_STATES_T, OWNER_T>
{
public:
    Action(CHAR_STATES_T actionState_, const Collider &hurtbox_, int anim_, StateMarker transitionableFrom_, OWNER_T &owner_, const InputResolver &inputResolver_) :
        GenericAction<CHAR_STATES_T, OWNER_T>(actionState_, owner_, anim_),
        m_hurtbox(hurtbox_),
        m_transitionableFrom(std::move(transitionableFrom_)),
        m_inputResolver(inputResolver_)
    {
    }

    inline virtual ORIENTATION isPossibleInDirection(int extendBuffer_, bool &isProceed_) const override
    {
        isProceed_ = false;
        auto orientation = ParentClass::m_owner.getOwnOrientation();
        const auto &inq = m_inputResolver.getInputQueue();
        auto currentState = ParentClass::m_owner.getCurrentActionState();

        if (ParentClass::m_ownState == currentState && ATTEMPT_PROCEED)
        {
            auto res = attemptInput<true, false>(m_cmpProcLeft, m_cmpProcRight, orientation, inq, extendBuffer_);
            if (res != ORIENTATION::UNSPECIFIED)
            {
                isProceed_ = true;
                return res;
            }
        }

        if (m_transitionableFrom[currentState])
            return attemptInput<REQUIRE_ALIGNMENT, FORCE_REALIGN>(m_cmpLeft, m_cmpRight, orientation, inq, extendBuffer_);

        return ORIENTATION::UNSPECIFIED;
    }

protected:
    using ParentClass = GenericAction<CHAR_STATES_T, OWNER_T>;
    const Collider m_hurtbox;
    StateMarker m_transitionableFrom;
    CMP_LEFT m_cmpLeft;
    CMP_RIGHT m_cmpRight;
    CMP_PROCEED_LEFT m_cmpProcLeft;
    CMP_PROCEED_RIGHT m_cmpProcRight;
    const InputResolver &m_inputResolver;
};

template<typename CHAR_STATES_T, typename OWNER_T>
class ActionFloat: public Action<CHAR_STATES_T, false, true, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle, OWNER_T>
{
public:
    ActionFloat(CHAR_STATES_T actionState_, const Collider &hurtbox_, int anim_, StateMarker transitionableFrom_, OWNER_T &owner_, const InputResolver &inputResolver_) :
        ParentAction(actionState_, hurtbox_, anim_, transitionableFrom_, owner_, inputResolver_)
    {
    }

    inline virtual void onUpdate() override
    {
        ParentAction::onUpdate();

        const auto &inq = ParentAction::m_inputResolver.getInputQueue();
        auto &vel = ParentAction::m_owner.accessVelocity();
        auto &inr = ParentAction::m_owner.accessInertia();

        if (m_driftLeftInput(inq, 0))
        {
            if (vel.x > -4.0f)
                vel.x -= 0.15f;
        }

        if (m_driftRightInput(inq, 0))
        {
            if (vel.x < 4.0f)
                vel.x += 0.15f;
        }

        if (ParentAction::m_owner.accessVelocity().y < 0 && m_driftUpInput(inq, 0))
        {
            if (ParentAction::m_owner.m_framesInState < 10.0f)
                vel.y -= 0.4f;
        }

        auto total = vel.x + inr.x;
        if (total > 0)
            ParentAction::m_owner.setOwnOrientation(ORIENTATION::RIGHT);
        else if (total < 0)
            ParentAction::m_owner.setOwnOrientation(ORIENTATION::LEFT);
    }

protected:
    using ParentAction = Action<CHAR_STATES_T, false, true, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle, OWNER_T>;
    InputComparatorHoldLeft m_driftLeftInput;
    InputComparatorHoldRight m_driftRightInput;
    InputComparatorHoldUp m_driftUpInput;

};

#endif

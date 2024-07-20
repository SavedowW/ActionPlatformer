#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "CoreComponents.h"
#include "InputResolver.h"
#include "InputComparators.h"
#include "CollisionArea.h"
#include "TimelineProperty.h"
#include "FrameTimer.h"
#include "StateMarker.h"

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
    GenericAction(CHAR_STATES_T ownState_, OWNER_T &owner_, int anim_, ComponentTransform &transform_,  ComponentPhysical &physical_, ComponentAnimationRenderable &renderable_) :
        m_ownState(ownState_),
        m_owner(owner_),
        m_anim(anim_),
        m_drag({1.0f, 0.0f}),
        m_appliedInertiaMultiplier({1.0f, 1.0f}),
        m_transform(transform_),
        m_physical(physical_),
        m_renderable(renderable_)
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

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setDrag(TimelineProperty<Vector2<float>> &&drag_)
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

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setUpdateSpeedLimitData(TimelineProperty<Vector2<float>> &&ownVelLimitUpd_, TimelineProperty<Vector2<float>> &&ownInrLimitUpd_)
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

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setCooldown(FrameTimer<true> *cooldown_, int cooldownTime_)
    {
        m_cooldown = cooldown_;
        m_cooldownTime = cooldownTime_;
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &setRecoveryFrames(TimelineProperty<StateMarker> &&recoveryFrames_)
    {
        m_recoveryFrames = std::move(recoveryFrames_);
        return *this;
    }

    inline GenericAction<CHAR_STATES_T, OWNER_T> &addTransitionAnim(CHAR_STATES_T oldState_, int anim_)
    {
        m_uniqueTransitionAnims[oldState_] = anim_;
        return *this;
    }

    inline virtual void onSwitchTo()
    {
        auto oldState = m_owner.m_currentAction->m_ownState;
        if (m_uniqueTransitionAnims.contains(oldState))
            m_renderable.m_currentAnimation = m_renderable.m_animations[m_uniqueTransitionAnims[oldState]].get();
        else
            m_renderable.m_currentAnimation = m_renderable.m_animations[m_anim].get();
        m_renderable.m_currentAnimation->reset();
        m_owner.m_currentAction = this;
        m_owner.m_framesInState = 0;

        if (m_convertVelocityOnSwitch)
            m_physical.velocityToInertia();

        if (m_setGroundedOnSwitch.isSet())
            m_owner.isGrounded = m_setGroundedOnSwitch;

        if (m_cooldown)
            m_cooldown->begin(m_cooldownTime);

        m_physical.m_pushbox = getHurtbox();
    }

    inline virtual void onUpdate()
    {
        if (m_usingUpdateMovement)
        {
            m_physical.m_velocity = m_physical.m_velocity.mulComponents(m_mulOwnVelUpd[m_owner.m_framesInState]) + m_transform.getOwnHorDir().mulComponents(m_mulOwnDirVelUpd[m_owner.m_framesInState]) + m_rawAddVelUpd[m_owner.m_framesInState];
            m_physical.m_inertia = m_physical.m_inertia.mulComponents(m_mulOwnInrUpd[m_owner.m_framesInState]) + m_transform.getOwnHorDir().mulComponents(m_mulOwnDirInrUpd[m_owner.m_framesInState]) + m_rawAddInrUpd[m_owner.m_framesInState];
        }

        if (!m_ownVelLimitUpd.isEmpty())
            m_physical.m_velocity = utils::clamp(m_physical.m_velocity, -m_ownVelLimitUpd[m_owner.m_framesInState], m_ownVelLimitUpd[m_owner.m_framesInState]);

        if (!m_ownInrLimitUpd.isEmpty())
            m_physical.m_inertia = utils::clamp(m_physical.m_inertia, -m_ownInrLimitUpd[m_owner.m_framesInState], m_ownInrLimitUpd[m_owner.m_framesInState]);

        if (m_transitionOnOutdated.isSet())
        {
            if (m_owner.m_framesInState >= m_duration)
                onOutdated();
        }
    }

    inline virtual void onOutdated()
    {
        m_owner.switchTo(m_transitionOnOutdated);
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

    virtual Vector2<float> getDrag(uint32_t currentFrame_) const
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

    bool canCancelInto(CHAR_STATES_T state_, uint32_t frame_) const
    {
        if (m_recoveryFrames.isEmpty())
            return false;
            
        return m_recoveryFrames[frame_].getMark((int)state_);
    }

    inline virtual ORIENTATION isPossibleInDirection(int extendBuffer_, bool &isProceed_) const = 0;

    inline virtual const Collider &getHurtbox() const = 0;

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

    TimelineProperty<Vector2<float>> m_drag;

    bool m_convertVelocityOnSwitch = false;

    TimelineProperty<Vector2<float>> m_ownVelLimitUpd;
    TimelineProperty<Vector2<float>> m_ownInrLimitUpd;

    TimelineProperty<float> m_magnetLimit;

    utils::OptionalProperty<CHAR_STATES_T> m_transitionOnOutdated;
    utils::OptionalProperty<uint32_t> m_duration;

    TimelineProperty<bool> m_canFallThrough;
    utils::OptionalProperty<bool> m_setGroundedOnSwitch;

    FrameTimer<true> *m_cooldown = nullptr;
    uint32_t m_cooldownTime = 0;

    TimelineProperty<StateMarker> m_recoveryFrames;

    std::map<CHAR_STATES_T, int> m_uniqueTransitionAnims;

    ComponentTransform &m_transform;
    ComponentPhysical &m_physical;
    ComponentAnimationRenderable &m_renderable;
};

template<typename CHAR_STATES_T, bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN,
    typename CMP_LEFT, typename CMP_RIGHT,
    bool ATTEMPT_PROCEED, typename CMP_PROCEED_LEFT, typename CMP_PROCEED_RIGHT,
    typename OWNER_T>
class PlayerAction : public GenericAction<CHAR_STATES_T, OWNER_T>
{
public:
    PlayerAction(CHAR_STATES_T actionState_, const Collider &hurtbox_, int anim_, StateMarker transitionableFrom_, OWNER_T &owner_, const InputResolver &inputResolver_, ComponentTransform &transform_, ComponentPhysical &physical_, ComponentAnimationRenderable &renderable_) :
        GenericAction<CHAR_STATES_T, OWNER_T>(actionState_, owner_, anim_, transform_, physical_, renderable_),
        m_hurtbox(hurtbox_),
        m_transitionableFrom(std::move(transitionableFrom_)),
        m_inputResolver(inputResolver_)
    {
    }

    inline virtual void onSwitchTo() override
    {
        if (m_realignOnSwitchForInput)
        {
            auto indir = m_inputResolver.getCurrentInputDir();
            if (indir.x > 0)
                ParentClass::m_transform.m_ownOrientation = ORIENTATION::RIGHT;
            else if (indir.x < 0)
                ParentClass::m_transform.m_ownOrientation = ORIENTATION::LEFT;
        }

        ParentClass::onSwitchTo();
    }

    inline virtual ORIENTATION isPossibleInDirection(int extendBuffer_, bool &isProceed_) const override
    {
        if (ParentClass::m_cooldown && ParentClass::m_cooldown->isActive())
            return ORIENTATION::UNSPECIFIED;

        auto &physical = ParentClass::m_owner.getComponent<ComponentPhysical>();

        isProceed_ = false;
        auto orientation = ParentClass::m_transform.m_ownOrientation;
        const auto &inq = m_inputResolver.getInputQueue();
        auto currentState = ParentClass::m_owner.getCurrentActionState();

        ORIENTATION SlopeDir = ORIENTATION::UNSPECIFIED;
        if (physical.m_onSlopeWithAngle > 0)
            SlopeDir = ORIENTATION::RIGHT;
        else if (physical.m_onSlopeWithAngle < 0)
            SlopeDir = ORIENTATION::LEFT;

        bool possibleToLeft = (!m_alignedSlopeMax.isSet() || physical.m_onSlopeWithAngle <= 0 || physical.m_onSlopeWithAngle <= m_alignedSlopeMax);
        bool possibleToRight = (!m_alignedSlopeMax.isSet() || physical.m_onSlopeWithAngle >= 0 || -physical.m_onSlopeWithAngle <= m_alignedSlopeMax);

        InputComparatorFail failin;

        if (ParentClass::m_ownState == currentState && ATTEMPT_PROCEED)
        {
            auto &lInput = (possibleToLeft ? static_cast<const InputComparator&>(m_cmpProcLeft) : static_cast<const InputComparator&>(failin));
            auto &rInput = (possibleToRight ? static_cast<const InputComparator&>(m_cmpProcRight) : static_cast<const InputComparator&>(failin));

            auto res = attemptInput<true, false>(lInput, rInput, orientation, inq, extendBuffer_);
            if (res != ORIENTATION::UNSPECIFIED)
            {
                isProceed_ = true;
                return res;
            }
        }

        auto &lInput = (possibleToLeft ? static_cast<const InputComparator&>(m_cmpLeft) : static_cast<const InputComparator&>(failin));
        auto &rInput = (possibleToRight ? static_cast<const InputComparator&>(m_cmpRight) : static_cast<const InputComparator&>(failin));

        if (m_transitionableFrom[currentState] || ParentClass::m_owner.getCurrentAction()->canCancelInto(ParentClass::m_ownState, ParentClass::m_owner.getFramesInState()))
            return attemptInput<REQUIRE_ALIGNMENT, FORCE_REALIGN>(lInput, rInput, orientation, inq, extendBuffer_);

        return ORIENTATION::UNSPECIFIED;
    }

    inline PlayerAction<CHAR_STATES_T, REQUIRE_ALIGNMENT, FORCE_REALIGN, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT, OWNER_T> 
        &setAlignedSlopeMax(float alignedSlopeMax_)
    {
        m_alignedSlopeMax = alignedSlopeMax_;
        return *this;
    }

    inline PlayerAction<CHAR_STATES_T, REQUIRE_ALIGNMENT, FORCE_REALIGN, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT, OWNER_T> 
        &setRealignOnSwitch(bool realignOnSwitch_)
    {
        m_realignOnSwitchForInput = realignOnSwitch_;
        return *this;
    }

    inline virtual const Collider &getHurtbox() const override
    {
        return m_hurtbox;
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

    utils::OptionalProperty<float> m_alignedSlopeMax;
    bool m_realignOnSwitchForInput = false;
};

template<typename CHAR_STATES_T, typename OWNER_T>
class ActionFloat: public PlayerAction<CHAR_STATES_T, false, true, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle, OWNER_T>
{
public:
    ActionFloat(CHAR_STATES_T actionState_, const Collider &hurtbox_, int anim_, StateMarker transitionableFrom_, OWNER_T &owner_, const InputResolver &inputResolver_, ComponentTransform &transform_, ComponentPhysical &physical_, ComponentAnimationRenderable &renderable_) :
        ParentAction(actionState_, hurtbox_, anim_, transitionableFrom_, owner_, inputResolver_, transform_, physical_, renderable_)
    {
    }

    inline virtual void onSwitchTo() override
    {
        ParentAction::onSwitchTo();
        if (ParentAction::m_owner.getComponent<ComponentObstacleFallthrough>().isIgnoringAllObstacles() && abs(ParentAction::m_physical.m_velocity.x) > 0.8f)
            ParentAction::m_physical.m_velocity.y += 5.0f;
    }

    inline virtual void onUpdate() override
    {
        ParentAction::onUpdate();

        const auto &inq = ParentAction::m_inputResolver.getInputQueue();

        if (m_driftLeftInput(inq, 0))
        {
            if (ParentAction::m_physical.m_velocity.x > -4.0f)
                ParentAction::m_physical.m_velocity.x -= 0.15f;
        }

        if (m_driftRightInput(inq, 0))
        {
            if (ParentAction::m_physical.m_velocity.x < 4.0f)
                ParentAction::m_physical.m_velocity.x += 0.15f;
        }

        if (ParentAction::m_physical.m_velocity.y < 0 && m_driftUpInput(inq, 0))
        {
            if (ParentAction::m_owner.m_framesInState < 10.0f)
                ParentAction::m_physical.m_velocity.y -= 0.4f;
        }

        auto total = ParentAction::m_physical.m_velocity.x + ParentAction::m_physical.m_inertia.x;
        if (total > 0)
            ParentAction::m_transform.m_ownOrientation = ORIENTATION::RIGHT;
        else if (total < 0)
            ParentAction::m_transform.m_ownOrientation = ORIENTATION::LEFT;
    }

protected:
    using ParentAction = PlayerAction<CHAR_STATES_T, false, true, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle, OWNER_T>;
    InputComparatorHoldLeft m_driftLeftInput;
    InputComparatorHoldRight m_driftRightInput;
    InputComparatorHoldUp m_driftUpInput;

};

template<typename CHAR_STATES_T, typename OWNER_T>
class WallClingAction: public PlayerAction<CHAR_STATES_T, false, true, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail, OWNER_T>
{
public:
    WallClingAction(CHAR_STATES_T actionState_, const Collider &hurtbox_, int anim_, StateMarker transitionableFrom_, OWNER_T &owner_, const InputResolver &inputResolver_, CHAR_STATES_T switchOnLeave_, ComponentTransform &transform_, ComponentPhysical &physical_, ComponentAnimationRenderable &renderable_) :
        ParentAction(actionState_, hurtbox_, anim_, transitionableFrom_, owner_, inputResolver_, transform_, physical_, renderable_),
        m_switchOnLeave(switchOnLeave_)
    {
        ParentGenericAction::setGravity(TimelineProperty<Vector2<float>>({0.0f, 0.020f}));
        ParentGenericAction::setConvertVelocityOnSwitch(true);
    }

    inline virtual ORIENTATION isPossibleInDirection(int extendBuffer_, bool &isProceed_) const override
    {
        auto &owner = ParentGenericAction::m_owner;
        auto currentState = owner.getCurrentActionState();

        if (ParentGenericAction::m_cooldown && ParentGenericAction::m_cooldown->isActive() ||
        (!ParentAction::m_transitionableFrom[currentState] && !owner.getCurrentAction()->canCancelInto(ParentGenericAction::m_ownState, owner.getFramesInState())))
            return ORIENTATION::UNSPECIFIED;        

        isProceed_ = false;
        auto orientation = ParentGenericAction::m_transform.m_ownOrientation;
        ParentAction::m_inputResolver.getInputQueue();
        const auto &inq = ParentAction::m_inputResolver.getInputQueue();

        auto *currentTrigger = ParentGenericAction::m_physical.m_collisionArea.getOverlappedTrigger(ParentGenericAction::m_physical.getPushbox(), Trigger::Tag::ClingArea);
        if (!currentTrigger)
            return ORIENTATION::UNSPECIFIED;

        bool possibleToLeft = (*currentTrigger & Trigger::Tag::LEFT);
        bool possibleToRight = (*currentTrigger & Trigger::Tag::RIGHT);
        if (!possibleToLeft && !possibleToRight)
            return ORIENTATION::UNSPECIFIED;

        InputComparatorFail failin;

        auto &lInput = (possibleToLeft ? static_cast<const InputComparator&>(ParentAction::m_cmpLeft) : static_cast<const InputComparator&>(failin));
        auto &rInput = (possibleToRight ? static_cast<const InputComparator&>(ParentAction::m_cmpRight) : static_cast<const InputComparator&>(failin));

        return attemptInput<false, true>(lInput, rInput, orientation, inq, extendBuffer_);

        return ORIENTATION::UNSPECIFIED;
    }

    inline virtual void onSwitchTo() override
    {
        ParentAction::onSwitchTo();

        auto &owner = ParentGenericAction::m_owner;
        if (ParentGenericAction::m_physical.m_inertia.y > 0)
            ParentGenericAction::m_physical.m_inertia.y = 0;
        if (ParentGenericAction::m_physical.m_velocity.y > 0)
            ParentGenericAction::m_physical.m_velocity.y = 0;

        ParentGenericAction::m_physical.m_velocity.x = 0;
        ParentGenericAction::m_physical.m_inertia.x = 0;

        auto pb = ParentGenericAction::m_physical.getPushbox();
        m_currentTrigger = ParentGenericAction::m_physical.m_collisionArea.getOverlappedTrigger(pb, Trigger::Tag::ClingArea);


        if (*m_currentTrigger & Trigger::Tag::LEFT)
            ParentGenericAction::m_transform.m_pos.x = m_currentTrigger->getRightEdge() - pb.m_halfSize.x;
        else if (*m_currentTrigger & Trigger::Tag::RIGHT)
            ParentGenericAction::m_transform.m_pos.x = m_currentTrigger->getLeftEdge() + pb.m_halfSize.x;
    }

    inline virtual void onUpdate() override
    {
        ParentAction::onUpdate();

        auto &owner = ParentGenericAction::m_owner;
        auto playercld = ParentGenericAction::m_physical.getPushbox();

        if (!m_currentTrigger->checkCollisionWith<true, false>(playercld))
        {
            if (playercld.getTopEdge() >= m_currentTrigger->getBottomEdge())
                owner.switchTo(m_switchOnLeave);
            else
                ParentGenericAction::m_transform.m_pos.y = m_currentTrigger->getTopEdge();
        }
    }

protected:
    using ParentGenericAction = GenericAction<CHAR_STATES_T, OWNER_T>;
    using ParentAction = PlayerAction<CHAR_STATES_T, false, true, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail, OWNER_T>;
    const Trigger *m_currentTrigger = nullptr;
    CHAR_STATES_T m_switchOnLeave;

};

template<typename CHAR_STATES_T, typename OWNER_T>
class WallClingPrejump: public PlayerAction<CHAR_STATES_T, true, false, InputComparatorTapAnyLeft, InputComparatorTapAnyRight, false, InputComparatorFail, InputComparatorFail, OWNER_T>
{
public:
    WallClingPrejump(CHAR_STATES_T actionState_, const Collider &hurtbox_, int anim_, StateMarker transitionableFrom_, OWNER_T &owner_, const InputResolver &inputResolver_, ComponentTransform &transform_, ComponentPhysical &physical_, ComponentAnimationRenderable &renderable_) :
        ParentAction(actionState_, hurtbox_, anim_, transitionableFrom_, owner_, inputResolver_, transform_, physical_, renderable_)
    {
        ParentGenericAction::setGravity(TimelineProperty<Vector2<float>>({0.0f, 0.020f}));
        ParentGenericAction::setConvertVelocityOnSwitch(true);
    }

    inline virtual void onSwitchTo() override
    {
        ParentAction::onSwitchTo();

        if (ParentGenericAction::m_physical.m_inertia.y > 0)
            ParentGenericAction::m_physical.m_inertia.y = 0;
        if (ParentGenericAction::m_physical.m_velocity.y > 0)
            ParentGenericAction::m_physical.m_velocity.y = 0;
    }

    inline virtual void onOutdated() override
    {
        ParentAction::m_inputResolver.getInputQueue();
        const auto &inq = ParentAction::m_inputResolver.getInputQueue();

        Vector2<float> targetSpeed;
        int orient = ParentGenericAction::m_transform.getOwnHorDir().x;

        bool upIn = m_u(inq, 0);
        bool sideIn = (orient > 0 ? m_r(inq, 0) : m_l(inq, 0));
        bool downIn = m_d(inq, 0);

        bool fall = false;

        if (upIn)
        {
            if (sideIn)
                targetSpeed = {orient * 5.0f, -5.5f};
            else
                targetSpeed = {orient * 1.0f, -6.5f};
        }
        else if (sideIn)
        {
            if (downIn)
                targetSpeed = {orient * 6.0f, 0.0f};
            else
                targetSpeed = {orient * 6.0f, -3.0f};
        }
        else
            fall = true;

        std::cout << targetSpeed << std::endl;

        if (fall)
        {
            ParentAction::m_physical.m_velocity = {0.0f, 0.1f};
            ParentAction::m_physical.m_inertia = {0.0f, 0.0f};
        }
        else
            ParentAction::m_physical.m_velocity += targetSpeed;

        ParentAction::onOutdated();
    }

protected:
    using ParentGenericAction = GenericAction<CHAR_STATES_T, OWNER_T>;
    using ParentAction = PlayerAction<CHAR_STATES_T, true, false, InputComparatorTapAnyLeft, InputComparatorTapAnyRight, false, InputComparatorFail, InputComparatorFail, OWNER_T>;

    InputComparatorHoldLeft m_l;
    InputComparatorHoldRight m_r;
    InputComparatorHoldUp m_u;
    InputComparatorHoldDown m_d;
};


template<typename CHAR_STATES_T, typename OWNER_T>
class MobAction : public GenericAction<CHAR_STATES_T, OWNER_T>
{
public:
    MobAction(CHAR_STATES_T actionState_, const Collider &hurtbox_, int anim_, StateMarker transitionableFrom_, OWNER_T &owner_,  ComponentTransform &transform_, ComponentPhysical &physical_, ComponentAnimationRenderable &renderable_) :
        GenericAction<CHAR_STATES_T, OWNER_T>(actionState_, owner_, anim_, transform_, physical_, renderable_),
        m_hurtbox(hurtbox_),
        m_transitionableFrom(std::move(transitionableFrom_))
    {
    }

    inline virtual ORIENTATION isPossibleInDirection(int extendBuffer_, bool &isProceed_) const override
    {
        isProceed_ = false;
        auto orientation = ParentClass::m_transform.m_ownOrientation;
        auto currentState = ParentClass::m_owner.getCurrentActionState();

        if (ParentClass::m_ownState == currentState)
        {
            isProceed_ = true;
            return orientation;
        }

        if (m_transitionableFrom[currentState])
            return orientation;
        else
            return ORIENTATION::UNSPECIFIED;
    }

    inline virtual const Collider &getHurtbox() const override
    {
        return m_hurtbox;
    }

protected:
    using ParentClass = GenericAction<CHAR_STATES_T, OWNER_T>;
    const Collider m_hurtbox;
    StateMarker m_transitionableFrom;
};

#endif

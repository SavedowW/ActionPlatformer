#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_
#include "TypeManip.hpp"
#include "Vector2.h"
#include "StateMarker.hpp"

template<typename Owner_t, typename PLAYER_STATE_T>
class GenericState;

template<typename Owner_t, typename PLAYER_STATE_T>
class StateMachine
{
protected:
    using OwnedState = GenericState<Owner_t, PLAYER_STATE_T>;

public:
    StateMachine() = default;
    StateMachine(const StateMachine &) = delete;
    StateMachine(StateMachine &&) = default;
    StateMachine &operator=(const StateMachine &) = delete;
    StateMachine &operator=(StateMachine &&) = default;

    virtual OwnedState *getRealCurrentState()
    {
        return m_currentState->getRealCurrentState();
    }

    inline void addState(std::unique_ptr<OwnedState> &&state_)
    {
        m_stateIds[state_->m_stateId] = m_states.size();
        state_->setParent(this);
        m_states.push_back(std::move(state_));
    }

    inline void setInitialState(PLAYER_STATE_T state_)
    {
        m_framesInState = 0;
        m_currentState = m_states[m_stateIds[state_]].get();
    }

    void switchCurrentState(Owner_t &owner_, OwnedState *state_)
    {
        m_currentState->leave(owner_, state_->m_stateId);
        state_->enter(owner_, m_currentState->m_stateId);
        m_currentState = state_;
        m_framesInState = 0;
    }

    void switchCurrentState(Owner_t &owner_, PLAYER_STATE_T stateId_)
    {
        switchCurrentState(owner_, m_states[m_stateIds[stateId_]].get());
    }

    inline virtual bool update(Owner_t &owner_, uint32_t currentFrame_)
    {
        if (m_currentState->update(owner_, m_framesInState))
        {
            if (!attemptTransition(owner_))
            {
                m_framesInState++;
            }
            else
                return true;
        }
        else
            m_framesInState++;

        return false;
    }

    bool attemptTransition(Owner_t &owner_)
    {
        auto currentStateId = m_currentState->m_stateId;
        for (auto &el : m_states)
        {
            if (!el->transitionableFrom(currentStateId))
                continue;

            auto res = el->isPossible(owner_);
            if (res != ORIENTATION::UNSPECIFIED)
            {
                auto &trans = std::get<ComponentTransform&>(owner_);
                trans.m_orientation = res;
                switchCurrentState(owner_, el.get());
                return true;
            }
        }

        return false;
    }

    virtual std::string getName() const
    {
        return std::string("root") + " -> " + m_currentState->getName(m_framesInState);
    }

    std::vector<std::unique_ptr<OwnedState>> m_states;
    std::unordered_map<PLAYER_STATE_T, int> m_stateIds;

    OwnedState *m_currentState = nullptr;
    uint32_t m_framesInState = 0;
};

template<typename Owner_t, typename PLAYER_STATE_T>
std::ostream &operator<<(std::ostream &os_, const StateMachine<Owner_t, PLAYER_STATE_T> &rhs_)
{
    os_ << rhs_.getName();
    return os_;
}

template<typename Owner_t, typename PLAYER_STATE_T>
class GenericState
{
protected:
    using ParentSM = StateMachine<Owner_t, PLAYER_STATE_T>;

public:
    GenericState(PLAYER_STATE_T stateId_, const std::string &stateName_, StateMarker<PLAYER_STATE_T> &&transitionableFrom_, int anim_) :
        m_stateId(stateId_),
        m_stateName(stateName_),
        m_transitionableFrom(std::move(transitionableFrom_)),
        m_anim(anim_),
        m_drag({1.0f, 0.0f}),
        m_appliedInertiaMultiplier({1.0f, 1.0f})
    {}

    virtual GenericState<Owner_t, PLAYER_STATE_T> *getRealCurrentState()
    {
        return this;
    }

    void setParent(ParentSM *parent_)
    {
        m_parent = parent_;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setTransitionOnTouchedGround(PLAYER_STATE_T state_)
    {
        m_transitionOnLand = state_;
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setTransitionOnLostGround(PLAYER_STATE_T state_)
    {
        m_transitionOnLostGround = state_;
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setGravity(TimelineProperty<Vector2<float>> &&gravity_)
    {
        m_gravity = std::move(gravity_);
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setDrag(TimelineProperty<Vector2<float>> &&drag_)
    {
        m_drag = std::move(drag_);
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setCanFallThrough(TimelineProperty<bool> &&fallThrough_)
    {
        m_canFallThrough = std::move(fallThrough_);
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setNoLanding(TimelineProperty<bool> &&noLanding_)
    {
        m_noUpwardLanding = std::move(noLanding_);
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>> &&inerMul_)
    {
        m_appliedInertiaMultiplier = std::move(inerMul_);
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setConvertVelocityOnSwitch(bool convertVelocity_)
    {
        m_convertVelocityOnSwitch = convertVelocity_;
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setOutdatedTransition(PLAYER_STATE_T state_, uint32_t duration_)
    {
        m_transitionOnOutdated = state_;
        m_duration = duration_;
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setUpdateMovementData(
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

    inline GenericState<Owner_t, PLAYER_STATE_T> &setMagnetLimit(TimelineProperty<float> &&magnetLimit_)
    {
        m_magnetLimit = std::move(magnetLimit_);
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setUpdateSpeedLimitData(TimelineProperty<Vector2<float>> &&ownVelLimitUpd_, TimelineProperty<Vector2<float>> &&ownInrLimitUpd_)
    {
        m_ownVelLimitUpd = std::move(ownVelLimitUpd_);
        m_ownInrLimitUpd = std::move(ownInrLimitUpd_);

        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setGroundedOnSwitch(bool isGrounded_)
    {
        m_setGroundedOnSwitch = isGrounded_;
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setCooldown(FrameTimer<true> *cooldown_, int cooldownTime_)
    {
        m_cooldown = cooldown_;
        m_cooldownTime = cooldownTime_;
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &setRecoveryFrames(TimelineProperty<StateMarker<PLAYER_STATE_T>> &&recoveryFrames_)
    {
        m_recoveryFrames = std::move(recoveryFrames_);
        return *this;
    }

    inline GenericState<Owner_t, PLAYER_STATE_T> &addTransitionAnim(PLAYER_STATE_T oldState_, int anim_)
    {
        m_uniqueTransitionAnims[oldState_] = anim_;
        return *this;
    }

    inline virtual void enter(Owner_t &owner_, PLAYER_STATE_T from_)
    {
        std::cout << "Switched to " << m_stateName << std::endl;

        auto &renderable = std::get<ComponentAnimationRenderable&>(owner_);
        auto &physical = std::get<ComponentPhysical&>(owner_);

        // Handle animation
        if (m_uniqueTransitionAnims.contains(from_))
            renderable.m_currentAnimation = renderable.m_animations[m_uniqueTransitionAnims[from_]].get();
        else
            renderable.m_currentAnimation = renderable.m_animations[m_anim].get();
        renderable.m_currentAnimation->reset();

        // Convert velocity
        if (m_convertVelocityOnSwitch)
            physical.velocityToInertia();

        // Force grounded
        if (m_setGroundedOnSwitch.isSet())
            physical.m_isGrounded = m_setGroundedOnSwitch;

        // set cooldown if necessary
        if (m_cooldown)
            m_cooldown->begin(m_cooldownTime);

        physical.m_pushbox = Collider{Vector2{0.0f, -30.0f}, Vector2{10.0f, 30.0f}}; // TODO: to property
    }

    inline virtual void leave(Owner_t &owner_, PLAYER_STATE_T to_)
    {
    }

    // True if its ok to attempt to change state
    inline virtual bool update(Owner_t &owner_, uint32_t currentFrame_)
    {
        auto &transform = std::get<ComponentTransform&>(owner_);
        auto &physical = std::get<ComponentPhysical&>(owner_);
        auto &animrnd = std::get<ComponentAnimationRenderable&>(owner_);

        animrnd.m_currentAnimation->update();

        // TODO: Magnet limit ?
        // TODO: recovery frames

        // Handle velocity and inertia changes
        if (m_usingUpdateMovement)
        {
            physical.m_velocity = physical.m_velocity.mulComponents(m_mulOwnVelUpd[currentFrame_]) + Vector2<int>(transform.m_orientation).mulComponents(m_mulOwnDirVelUpd[currentFrame_]) + m_rawAddVelUpd[currentFrame_];
            physical.m_inertia = physical.m_inertia.mulComponents(m_mulOwnInrUpd[currentFrame_]) + Vector2<int>(transform.m_orientation).mulComponents(m_mulOwnDirInrUpd[currentFrame_]) + m_rawAddInrUpd[currentFrame_];
        }

        // Handle gravity
        physical.m_gravity = m_gravity[currentFrame_];
        physical.m_drag = m_drag[currentFrame_];
        physical.m_inertiaMultiplier = m_appliedInertiaMultiplier[currentFrame_];

        // Handle velocity and inertia limits
        if (!m_ownVelLimitUpd.isEmpty())
            physical.m_velocity = utils::clamp(physical.m_velocity, -m_ownVelLimitUpd[currentFrame_], m_ownVelLimitUpd[currentFrame_]);

        if (!m_ownInrLimitUpd.isEmpty())
            physical.m_inertia = utils::clamp(physical.m_inertia, -m_ownInrLimitUpd[currentFrame_], m_ownInrLimitUpd[currentFrame_]);

        // Handle duration
        if (m_transitionOnOutdated.isSet())
        {
            if (currentFrame_ >= m_duration)
                onOutdated(owner_);
        }

        physical.m_noUpwardLanding = m_noUpwardLanding[currentFrame_];
        physical.m_magnetLimit = m_magnetLimit[currentFrame_];

        return true;
    }

    inline virtual void onOutdated(Owner_t &owner_)
    {
        m_parent->switchCurrentState(owner_, m_transitionOnOutdated);
    }

    inline virtual void onTouchedGround(Owner_t &owner_)
    {
        if (m_transitionOnLand.isSet())
        {
            m_parent->switchCurrentState(owner_, m_transitionOnLand);
        }
    }

    inline virtual void onLostGround(Owner_t &owner_)
    {
        if (m_transitionOnLostGround.isSet())
        {
            m_parent->switchCurrentState(owner_, m_transitionOnLostGround);
        }
    }

    inline virtual ORIENTATION isPossible(Owner_t &owner_) const
    {
        if (m_cooldown && m_cooldown->isActive())
            return ORIENTATION::UNSPECIFIED;

        return std::get<ComponentTransform&>(owner_).m_orientation;
    }

    virtual std::string getName(uint32_t framesInState_) const
    {
        return m_stateName + " (" + std::to_string(framesInState_) + ")";
    }

    bool transitionableFrom(PLAYER_STATE_T state_) const
    {
        return m_transitionableFrom[state_];
    }

    const PLAYER_STATE_T m_stateId;

protected:
    const StateMarker<PLAYER_STATE_T> m_transitionableFrom;
    std::string m_stateName;
    ParentSM *m_parent = nullptr;

    int m_anim;
    utils::OptionalProperty<PLAYER_STATE_T> m_transitionOnLand;
    utils::OptionalProperty<PLAYER_STATE_T> m_transitionOnLostGround;
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

    utils::OptionalProperty<PLAYER_STATE_T> m_transitionOnOutdated;
    utils::OptionalProperty<uint32_t> m_duration;

    TimelineProperty<bool> m_canFallThrough;
    utils::OptionalProperty<bool> m_setGroundedOnSwitch;

    FrameTimer<true> *m_cooldown = nullptr;
    uint32_t m_cooldownTime = 0;

    TimelineProperty<StateMarker<PLAYER_STATE_T>> m_recoveryFrames;

    std::map<PLAYER_STATE_T, int> m_uniqueTransitionAnims;

    TimelineProperty<bool> m_noUpwardLanding;
};

template<typename Owner_t, typename PLAYER_STATE_T>
class NodeState: public StateMachine<Owner_t, PLAYER_STATE_T>, public GenericState<Owner_t, PLAYER_STATE_T>
{
public:
    NodeState(PLAYER_STATE_T stateId_, const std::string &stateName_, StateMarker<PLAYER_STATE_T> &&transitionableFrom_) :
        GenericState<Owner_t, PLAYER_STATE_T>(stateId_, stateName_, std::move(transitionableFrom_))
    {}

    virtual std::string getName(uint32_t framesInState_) const override
    {
        return std::string(GenericState<Owner_t, PLAYER_STATE_T>::m_stateName) + " (" + std::to_string(framesInState_) + ") -> " + StateMachine<Owner_t, PLAYER_STATE_T>::m_currentState->getName(StateMachine<Owner_t, PLAYER_STATE_T>::m_framesInState);
    }

    inline virtual bool update(Owner_t &owner_, uint32_t currentFrame_) override
    {
        GenericState<Owner_t, PLAYER_STATE_T>::update(owner_, currentFrame_);
        return StateMachine<Owner_t, PLAYER_STATE_T>::update(owner_, currentFrame_);
    }

    virtual GenericState<Owner_t, PLAYER_STATE_T> *getRealCurrentState() override
    {
        return StateMachine<Owner_t, PLAYER_STATE_T>::m_currentState->getRealCurrentState();
    }


private:

};

#endif
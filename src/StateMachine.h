#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_
#include "Vector2.h"
#include "StateMarker.hpp"
#include "World.h"
#include "CoreComponents.h"

using CharState = int;

class GenericState;

class StateMachine
{
public:
    StateMachine() = default;
    StateMachine(const StateMachine &) = delete;
    StateMachine(StateMachine &&) = default;
    StateMachine &operator=(const StateMachine &) = delete;
    StateMachine &operator=(StateMachine &&) = default;

    virtual GenericState *getRealCurrentState();

    void addState(std::unique_ptr<GenericState> &&state_);
    void switchCurrentState(EntityAnywhere owner_, GenericState *state_);
    bool attemptTransition(EntityAnywhere owner_, GenericState* until_);

    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_);
    virtual std::string getName() const;

    template<typename PLAYER_STATE_T>
    void switchCurrentState(EntityAnywhere owner_, PLAYER_STATE_T stateId_)
    {
        switchCurrentState(owner_, m_states[m_stateIds[static_cast<CharState>(stateId_)]].get());
    }
    
    template<typename PLAYER_STATE_T>
    inline void setInitialState(PLAYER_STATE_T state_)
    {
        auto initstate = static_cast<CharState>(state_);
        m_framesInState = 0;
        m_currentState = m_states[m_stateIds[initstate]].get();
    }

    std::vector<std::unique_ptr<GenericState>> m_states;
    std::unordered_map<CharState, int> m_stateIds;

    GenericState *m_currentState = nullptr;
    uint32_t m_framesInState = 0;
};

std::ostream &operator<<(std::ostream &os_, const StateMachine &rhs_);

class GenericState
{
public:
    template<typename PLAYER_STATE_T>
    GenericState(PLAYER_STATE_T stateId_, const std::string &stateName_, StateMarker &&transitionableFrom_, int anim_) :
        m_stateId(static_cast<CharState>(stateId_)),
        m_stateName(stateName_),
        m_transitionableFrom(std::move(transitionableFrom_)),
        m_anim(anim_),
        m_drag({1.0f, 0.0f}),
        m_appliedInertiaMultiplier({1.0f, 1.0f})
    {}

    void setParent(StateMachine *parent_);

    virtual GenericState *getRealCurrentState();

    template<typename PLAYER_STATE_T>
    GenericState &setTransitionOnTouchedGround(PLAYER_STATE_T state_)
    {
        m_transitionOnLand = static_cast<CharState>(state_);
        return *this;
    }

    template<typename PLAYER_STATE_T>
    inline GenericState &setTransitionOnLostGround(PLAYER_STATE_T state_)
    {
        m_transitionOnLostGround = static_cast<CharState>(state_);
        return *this;
    }

    template<typename PLAYER_STATE_T>
    GenericState &setOutdatedTransition(PLAYER_STATE_T state_, uint32_t duration_)
    {
        m_transitionOnOutdated = static_cast<CharState>(state_);
        m_duration = duration_;
        return *this;
    }

    template<typename PLAYER_STATE_T>
    GenericState &addTransitionAnim(PLAYER_STATE_T oldState_, int anim_)
    {
        m_uniqueTransitionAnims[static_cast<CharState>(oldState_)] = anim_;
        return *this;
    }

    GenericState &setGravity(TimelineProperty<Vector2<float>> &&gravity_);
    GenericState &setDrag(TimelineProperty<Vector2<float>> &&drag_);
    GenericState &setCanFallThrough(TimelineProperty<bool> &&fallThrough_);
    GenericState &setNoLanding(TimelineProperty<bool> &&noLanding_);
    GenericState &setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>> &&inerMul_);
    GenericState &setConvertVelocityOnSwitch(bool convertVelocity_);
    GenericState &setUpdateMovementData(
        TimelineProperty<Vector2<float>> &&mulOwnVelUpd_, TimelineProperty<Vector2<float>> &&mulOwnDirVelUpd_, TimelineProperty<Vector2<float>> &&rawAddVelUpd_,
        TimelineProperty<Vector2<float>> &&mulOwnInrUpd_, TimelineProperty<Vector2<float>> &&mulOwnDirInrUpd_, TimelineProperty<Vector2<float>> &&rawAddInrUpd_);
    GenericState &setMagnetLimit(TimelineProperty<float> &&magnetLimit_);
    GenericState &setUpdateSpeedLimitData(TimelineProperty<Vector2<float>> &&ownVelLimitUpd_, TimelineProperty<Vector2<float>> &&ownInrLimitUpd_);
    GenericState &setGroundedOnSwitch(bool isGrounded_);
    GenericState &setCooldown(FrameTimer<true> *cooldown_, int cooldownTime_);
    GenericState &setRecoveryFrames(TimelineProperty<StateMarker> &&recoveryFrames_);

    virtual void enter(EntityAnywhere owner_, CharState from_);
    virtual void leave(EntityAnywhere owner_, CharState to_);
    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_);
    virtual ORIENTATION isPossible(EntityAnywhere owner_) const;
    virtual std::string getName(uint32_t framesInState_) const;

    template<typename PLAYER_STATE_T>
    bool transitionableFrom(PLAYER_STATE_T state_) const
    {
        return m_transitionableFrom[state_];
    }

    virtual void onOutdated(EntityAnywhere owner_);
    virtual void onTouchedGround(EntityAnywhere owner_);
    virtual void onLostGround(EntityAnywhere owner_);

    const CharState m_stateId;

protected:
    const StateMarker m_transitionableFrom;
    std::string m_stateName;
    StateMachine *m_parent = nullptr;

    int m_anim;
    utils::OptionalProperty<CharState> m_transitionOnLand;
    utils::OptionalProperty<CharState> m_transitionOnLostGround;
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

    utils::OptionalProperty<CharState> m_transitionOnOutdated; // TODO: to std::optional
    utils::OptionalProperty<uint32_t> m_duration;

    TimelineProperty<bool> m_canFallThrough;
    utils::OptionalProperty<bool> m_setGroundedOnSwitch;

    FrameTimer<true> *m_cooldown = nullptr;
    uint32_t m_cooldownTime = 0;

    TimelineProperty<StateMarker> m_recoveryFrames;

    std::map<CharState, int> m_uniqueTransitionAnims;

    TimelineProperty<bool> m_noUpwardLanding;
};

/*
    Probably not very useful because you most likely will want to use different base classes
*/
class NodeState: public StateMachine, public GenericState
{
public:
    template<typename PLAYER_STATE_T>
    NodeState(PLAYER_STATE_T stateId_, const std::string &stateName_, StateMarker &&transitionableFrom_) :
        GenericState(stateId_, stateName_, std::move(transitionableFrom_))
    {}

    virtual std::string getName(uint32_t framesInState_) const override;
    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;
    virtual GenericState *getRealCurrentState() override;
};

#endif
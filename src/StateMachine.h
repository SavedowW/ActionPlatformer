#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_
#include "Core/Vector2.hpp"
#include "Core/StateMarker.hpp"
#include "World.h"
#include "Hit.h"
#include "Core/CoreComponents.h"
#include "Core/StaticMapping.hpp"
#include "Core/StateCommon.h"

//TODO: split, move general stuff into core

class GenericState;

enum class TraverseTraits : Traverse::TraitT {
    WALK = 0b001,
    JUMP = 0b010,
    FALL = 0b100
};

class StateMachine
{
public:
    StateMachine() = default;
    StateMachine(const StateMachine &) = delete;
    StateMachine(StateMachine &&) noexcept = default;
    StateMachine &operator=(const StateMachine &) = delete;
    StateMachine &operator=(StateMachine &&) noexcept = default;

    virtual GenericState *getRealCurrentState();

    template<typename T, typename... Args>
    T &addState(Args&&... args_);
    void switchCurrentState(EntityAnywhere owner_, GenericState *state_);
    bool attemptTransition(EntityAnywhere owner_, GenericState* until_);

    virtual bool update(EntityAnywhere owner_, const Time::NS&);
    virtual std::string getName() const;

    template<typename PLAYER_STATE_T>
    void switchCurrentState(EntityAnywhere owner_, PLAYER_STATE_T stateId_);

    void switchCurrentState(EntityAnywhere owner_, const std::vector<CharState>::iterator &current_,
        const std::vector<CharState>::iterator &end_);
    
    template<typename PLAYER_STATE_T>
    void setInitialState(PLAYER_STATE_T state_);

    virtual ~StateMachine() = default;

    std::vector<std::unique_ptr<GenericState>> m_states;
    std::unordered_map<CharState, size_t> m_stateIds;

    GenericState *m_currentState = nullptr;
    Time::NS m_timeInState{0};
};

std::ostream &operator<<(std::ostream &os_, const StateMachine &rhs_);

class GenericState
{
public:
    template<typename PLAYER_STATE_T>
    GenericState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_) :
        m_stateId(static_cast<CharState>(stateId_)),
        m_transitionableFrom(std::move(transitionableFrom_)),
        m_stateName(serialize<PLAYER_STATE_T>(stateId_))
    {}

    GenericState &operator=(const GenericState&) = delete;
    GenericState &operator=(GenericState&&) = delete;

    void setParent(StateMachine *parent_);

    virtual GenericState *getRealCurrentState();

    template<typename PLAYER_STATE_T>
    GenericState &setOutdatedTransition(PLAYER_STATE_T state_, const Time::NS &duration_);

    GenericState &setParticlesSingle(TimelineProperty<Time::NS, ParticleTemplate> &&particlesSingle_);
    GenericState &setParticlesLoopable(TimelineProperty<Time::NS, ParticleTemplate> &&particlesLoopable_, const Time::NS &duration_);

    virtual void enter(EntityAnywhere owner_, CharState from_);
    virtual void leave(EntityAnywhere owner_, CharState to_);

    // Return true if it allows to enter lower priority states
    virtual bool update(EntityAnywhere owner_, const Time::NS &timeInState_);
    
    virtual ORIENTATION isPossible(EntityAnywhere owner_) const;
    virtual std::string getName(const Time::NS &timeInState) const;
    bool transitionableFrom(CharState targetStateId_) const;
    virtual bool transitionableInto(CharState targetStateId_, const Time::NS &timeInState_) const;

    virtual void onOutdated(EntityAnywhere owner_);

    const CharState m_stateId;

    virtual ~GenericState() = default;

protected:
    void spawnParticle(EntityAnywhere owner_, const ParticleTemplate &partemplate_, const ComponentTransform &trans_, const ComponentPhysical &phys_, World &world_, SDL_FlipMode verFlip_);

    const StateMarker m_transitionableFrom;
    std::string m_stateName;
    StateMachine *m_parent = nullptr;

    std::optional<CharState> m_transitionOnOutdated;
    std::optional<Time::NS> m_duration;

    // TODO: trigger only once
    TimelineProperty<Time::NS, ParticleTemplate> m_particlesSingle;
    TimelineProperty<Time::NS, ParticleTemplate> m_particlesLoopable;
    Time::NS m_loopDuration{1};

    std::vector<entt::entity> m_lifetimeTiedParticles;
};

class PhysicalState: public GenericState
{
public:
    template<typename PLAYER_STATE_T>
    PhysicalState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_, ResID anim_);

    void leave(EntityAnywhere owner_, CharState to_) override;

    template<typename PLAYER_STATE_T>
    PhysicalState &addTransitionOnTouchedGround(const Time::NS &sinceTime_, PLAYER_STATE_T transition_)
    {
        m_transitionsOnLand.addPair(sinceTime_, std::optional<CharState>(static_cast<CharState>(transition_)));
        return *this;
    }

    void enter(EntityAnywhere owner_, CharState from_) override;
    bool update(EntityAnywhere owner_, const Time::NS &timeInState_) override;

    template<typename PLAYER_STATE_T>
    inline PhysicalState &setTransitionOnLostGround(PLAYER_STATE_T state_);

    template<typename PLAYER_STATE_T>
    PhysicalState &addTransitionAnim(PLAYER_STATE_T oldState_, ResID anim_);

    void updateActor(BattleActor &battleActor_) const;

    PhysicalState &setGravity(TimelineProperty<Time::NS, Vector2<float>> &&gravity_);
    PhysicalState &setDrag(TimelineProperty<Time::NS, Vector2<float>> &&drag_);
    PhysicalState &setMulInsidePushbox(TimelineProperty<Time::NS, std::optional<Vector2<float>>> &&mulInsidePushbox_);
    PhysicalState &setCanFallThrough(TimelineProperty<Time::NS, bool> &&fallThrough_);
    PhysicalState &setNoLanding(TimelineProperty<Time::NS, bool> &&noLanding_);
    PhysicalState &setAppliedInertiaMultiplier(TimelineProperty<Time::NS, Vector2<float>> &&inerMul_);
    PhysicalState &setTransitionVelocityMultiplier(TimelineProperty<Time::NS, Vector2<float>> &&convRate_);
    PhysicalState &setConvertVelocityOnSwitch(bool convertVelocity_, bool convertEnforced_);
    PhysicalState &setUpdateMovementData(
        TimelineProperty<Time::NS, Vector2<float>> &&mulOwnVelUpd_, TimelineProperty<Time::NS, Vector2<float>> &&mulOwnDirVelUpd_, TimelineProperty<Time::NS, Vector2<float>> &&rawAddVelUpd_,
        TimelineProperty<Time::NS, Vector2<float>> &&mulOwnInrUpd_, TimelineProperty<Time::NS, Vector2<float>> &&mulOwnDirInrUpd_, TimelineProperty<Time::NS, Vector2<float>> &&rawAddInrUpd_);
    PhysicalState &setMagnetLimit(TimelineProperty<Time::NS, unsigned int> &&magnetLimit_);
    PhysicalState &setUpdateSpeedLimitData(TimelineProperty<Time::NS, Vector2<float>> &&ownVelLimitUpd_, TimelineProperty<Time::NS, Vector2<float>> &&ownInrLimitUpd_);
    PhysicalState &setRecoveryFrames(TimelineProperty<Time::NS, StateMarker> &&recoveryFrames_);
    PhysicalState &setHurtboxes(Hurtbox &&hurtboxes_);
    PhysicalState &addHit(HitboxGroup &&hit_);
    PhysicalState &setHitStateMapping(HitStateMapping &&hitStateMapping_);
    bool transitionableInto(CharState targetStateId_, const Time::NS &timeInState_) const override;


    virtual void onTouchedGround(EntityAnywhere owner_);
    virtual void onLostGround(EntityAnywhere owner_);

protected:
    ResID m_anim;
    TimelineProperty<Time::NS, std::optional<CharState>> m_transitionsOnLand;
    std::optional<CharState> m_transitionOnLostGround;
    TimelineProperty<Time::NS, Vector2<float>> m_gravity;

    bool m_usingUpdateMovement = false;
    TimelineProperty<Time::NS, Vector2<float>> m_mulOwnVelUpd;
    TimelineProperty<Time::NS, Vector2<float>> m_mulOwnDirVelUpd;
    TimelineProperty<Time::NS, Vector2<float>> m_rawAddVelUpd;
    TimelineProperty<Time::NS, Vector2<float>> m_mulOwnInrUpd;
    TimelineProperty<Time::NS, Vector2<float>> m_mulOwnDirInrUpd;
    TimelineProperty<Time::NS, Vector2<float>> m_rawAddInrUpd;
    TimelineProperty<Time::NS, Vector2<float>> m_appliedInertiaMultiplier;
    TimelineProperty<Time::NS, Vector2<float>> m_drag;

    TimelineProperty<Time::NS, std::optional<Vector2<float>>> m_mulInsidePushbox;
    TimelineProperty<Time::NS, Vector2<float>> m_transitionVelocityMultiplier;

    bool m_convertVelocityOnSwitch = false;

    TimelineProperty<Time::NS, Vector2<float>> m_ownVelLimitUpd;
    TimelineProperty<Time::NS, Vector2<float>> m_ownInrLimitUpd;

    TimelineProperty<Time::NS, unsigned int> m_magnetLimit;

    TimelineProperty<Time::NS, bool> m_canFallThrough;

    TimelineProperty<Time::NS, StateMarker> m_recoveryFrames;

    std::map<CharState, ResID> m_uniqueTransitionAnims;

    TimelineProperty<Time::NS, bool> m_noLanding;
    bool m_convertEnforcedVelocity = false;

    bool m_hasHurtboxes = false;
    Hurtbox m_hurtboxes;

    std::vector<HitboxGroup> m_hits;

    std::optional<HitStateMapping> m_hitStateMapping;
};

/*
    Probably not very useful because you most likely will want to use different base classes
*/
class NodeState: public StateMachine, public GenericState
{
public:
    template<typename PLAYER_STATE_T>
    NodeState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_);

    std::string getName(const Time::NS &timeInState_) const override;
    bool update(EntityAnywhere owner_, const Time::NS &timeInState_) override;
    GenericState *getRealCurrentState() override;

private:
    using StateMachine::getName;
};

#endif
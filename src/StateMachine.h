#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_
#include "Vector2.h"
#include "StateMarker.hpp"
#include "World.h"
#include "Hit.h"
#include "CoreComponents.h"
#include "EnumMapping.hpp"
#include "StateCommon.h"

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
        if (static_cast<CharState>(stateId_) != std::numeric_limits<CharState>::max())
            switchCurrentState(owner_, m_states[m_stateIds[static_cast<CharState>(stateId_)]].get());
    }

    void switchCurrentState(EntityAnywhere owner_, const std::vector<CharState>::iterator &current_,
        const std::vector<CharState>::iterator &end_);
    
    template<typename PLAYER_STATE_T>
    inline void setInitialState(PLAYER_STATE_T state_)
    {
        auto initstate = static_cast<CharState>(state_);
        m_framesInState = 0;
        m_currentState = m_states[m_stateIds[initstate]].get();
    }

    std::vector<std::unique_ptr<GenericState>> m_states;
    std::unordered_map<CharState, size_t> m_stateIds;

    GenericState *m_currentState = nullptr;
    uint32_t m_framesInState = 0;
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

    void setParent(StateMachine *parent_);

    virtual GenericState *getRealCurrentState();

    template<typename PLAYER_STATE_T>
    GenericState &setOutdatedTransition(PLAYER_STATE_T state_, uint32_t duration_)
    {
        m_transitionOnOutdated = static_cast<CharState>(state_);
        m_duration = duration_;
        return *this;
    }

    GenericState &setParticlesSingle(TimelineProperty<ParticleTemplate> &&particlesSingle_);
    GenericState &setParticlesLoopable(TimelineProperty<ParticleTemplate> &&particlesLoopable_, uint32_t loopDuration_);

    virtual void enter(EntityAnywhere owner_, CharState from_);
    virtual void leave(EntityAnywhere owner_, CharState to_);

    // Return true if it allows to enter lower priority states
    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_);
    
    virtual ORIENTATION isPossible(EntityAnywhere owner_) const;
    virtual std::string getName(uint32_t framesInState_) const;
    bool transitionableFrom(CharState targetStateId_) const;
    virtual bool transitionableInto(CharState targetStateId_, uint32_t currentFrame_) const;

    virtual void onOutdated(EntityAnywhere owner_);

    const CharState m_stateId;

    virtual ~GenericState() = default;

protected:
    void spawnParticle(EntityAnywhere owner_, const ParticleTemplate &partemplate_, const ComponentTransform &trans_, const ComponentPhysical &phys_, World &world_, SDL_FlipMode verFlip_);

    const StateMarker m_transitionableFrom;
    std::string m_stateName;
    StateMachine *m_parent = nullptr;

    std::optional<CharState> m_transitionOnOutdated;
    std::optional<uint32_t> m_duration;

    TimelineProperty<ParticleTemplate> m_particlesSingle;
    TimelineProperty<ParticleTemplate> m_particlesLoopable;
    uint32_t m_loopDuration = 1;

    std::vector<entt::entity> m_lifetimeTiedParticles;
};

class PhysicalState: public GenericState
{
public:
    template<typename PLAYER_STATE_T>
    PhysicalState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_, ResID anim_) :
        GenericState(stateId_, std::move(transitionableFrom_)),
        m_anim(anim_),
        m_appliedInertiaMultiplier(Vector2{1.0f, 1.0f}),
        m_drag(Vector2{1.0f, 0.0f}),
        m_transitionVelocityMultiplier(Vector2{1.0f, 1.0f}),
        m_canFallThrough(true)
    {}

    virtual void leave(EntityAnywhere owner_, CharState to_) override;

    template<typename PLAYER_STATE_T>
    PhysicalState &addTransitionOnTouchedGround(uint32_t sinceFrame_, PLAYER_STATE_T transition_)
    {
        m_transitionsOnLand.addPair(sinceFrame_, std::optional<CharState>(static_cast<CharState>(transition_)));
        return *this;
    }

    virtual void enter(EntityAnywhere owner_, CharState from_) override;
    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

    template<typename PLAYER_STATE_T>
    inline PhysicalState &setTransitionOnLostGround(PLAYER_STATE_T state_)
    {
        m_transitionOnLostGround = static_cast<CharState>(state_);
        return *this;
    }

    template<typename PLAYER_STATE_T>
    PhysicalState &addTransitionAnim(PLAYER_STATE_T oldState_, ResID anim_)
    {
        m_uniqueTransitionAnims[static_cast<CharState>(oldState_)] = anim_;
        return *this;
    }

    void updateActor(BattleActor &battleActor_) const;

    PhysicalState &setGravity(TimelineProperty<Vector2<float>> &&gravity_);
    PhysicalState &setDrag(TimelineProperty<Vector2<float>> &&drag_);
    PhysicalState &setMulInsidePushbox(TimelineProperty<std::optional<Vector2<float>>> &&mulInsidePushbox_);
    PhysicalState &setCanFallThrough(TimelineProperty<bool> &&fallThrough_);
    PhysicalState &setNoLanding(TimelineProperty<bool> &&noLanding_);
    PhysicalState &setAppliedInertiaMultiplier(TimelineProperty<Vector2<float>> &&inerMul_);
    PhysicalState &setTransitionVelocityMultiplier(TimelineProperty<Vector2<float>> &&convRate_);
    PhysicalState &setConvertVelocityOnSwitch(bool convertVelocity_, bool convertEnforced_);
    PhysicalState &setUpdateMovementData(
        TimelineProperty<Vector2<float>> &&mulOwnVelUpd_, TimelineProperty<Vector2<float>> &&mulOwnDirVelUpd_, TimelineProperty<Vector2<float>> &&rawAddVelUpd_,
        TimelineProperty<Vector2<float>> &&mulOwnInrUpd_, TimelineProperty<Vector2<float>> &&mulOwnDirInrUpd_, TimelineProperty<Vector2<float>> &&rawAddInrUpd_);
    PhysicalState &setMagnetLimit(TimelineProperty<unsigned int> &&magnetLimit_);
    PhysicalState &setUpdateSpeedLimitData(TimelineProperty<Vector2<float>> &&ownVelLimitUpd_, TimelineProperty<Vector2<float>> &&ownInrLimitUpd_);
    PhysicalState &setCooldown(FrameTimer<true> *cooldown_, int cooldownTime_);
    PhysicalState &setRecoveryFrames(TimelineProperty<StateMarker> &&recoveryFrames_);
    PhysicalState &setHurtboxes(Hurtbox &&hurtboxes_);
    PhysicalState &addHit(HitboxGroup &&hit_);
    PhysicalState &setHitStateMapping(HitStateMapping &&hitStateMapping_);
    virtual bool transitionableInto(CharState targetStateId_, uint32_t currentFrame_) const override;


    virtual void onTouchedGround(EntityAnywhere owner_);
    virtual void onLostGround(EntityAnywhere owner_);

protected:
    ResID m_anim;
    TimelineProperty<std::optional<CharState>> m_transitionsOnLand;
    std::optional<CharState> m_transitionOnLostGround;
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

    TimelineProperty<std::optional<Vector2<float>>> m_mulInsidePushbox;
    TimelineProperty<Vector2<float>> m_transitionVelocityMultiplier;

    bool m_convertVelocityOnSwitch = false;

    TimelineProperty<Vector2<float>> m_ownVelLimitUpd;
    TimelineProperty<Vector2<float>> m_ownInrLimitUpd;

    TimelineProperty<unsigned int> m_magnetLimit;

    TimelineProperty<bool> m_canFallThrough;

    FrameTimer<true> *m_cooldown = nullptr;
    uint32_t m_cooldownTime = 0;

    TimelineProperty<StateMarker> m_recoveryFrames;

    std::map<CharState, ResID> m_uniqueTransitionAnims;

    TimelineProperty<bool> m_noLanding;
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
    NodeState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_) :
        GenericState(stateId_, std::move(transitionableFrom_))
    {}

    virtual std::string getName(uint32_t framesInState_) const override;
    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;
    virtual GenericState *getRealCurrentState() override;
};

#endif
#pragma once
#include "SM/StateMachine.h"
#include <optional>

struct ComponentAI
{
    //StateMachine m_sm;
    std::optional<SM::StateID> m_requestedState;
    ORIENTATION m_requestedOrientation = ORIENTATION::UNSPECIFIED;
    Vector2<float> m_navigationTarget;
    bool m_isNavigating = false;
    float m_additionalAccel = 0.0f;
    entt::entity m_chaseTarget;
    bool m_allowLeaveState = true;
};

#if 0

class AIState: public GenericState
{
public:
    template<typename PLAYER_STATE_T>
    AIState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_) :
        GenericState(stateId_, std::move(transitionableFrom_))
    {}

    void enter(EntityAnywhere owner_, SM::StateID from_) override;

    AIState &setEnterRequestedState(std::optional<SM::StateID> m_enterRequestedState_);
    AIState &setEnterRequestedOrientation(std::optional<ORIENTATION> enterRequestedOrientation_);

protected:
    std::optional<SM::StateID> m_enterRequestedState;
    std::optional<ORIENTATION> m_enterRequestedOrientation;
};

class AIStateNull: public GenericState
{
public:
    template<typename PLAYER_STATE_T>
    AIStateNull(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_) :
        GenericState(stateId_, std::move(transitionableFrom_))
    {}

    void enter(EntityAnywhere owner_, SM::StateID from_) override;

protected:
    std::optional<SM::StateID> m_enterRequestedState;
    std::optional<ORIENTATION> m_enterRequestedOrientation;
};

class RandomRoamState: public NodeState
{
public:
    template<typename PLAYER_STATE_T>
    RandomRoamState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_,
    PLAYER_STATE_T idle_, PLAYER_STATE_T walk_, std::pair<uint32_t, uint32_t> idleDurationRange_, std::pair<uint32_t, uint32_t> walkDurationRange_) :
        NodeState(stateId_, std::move(transitionableFrom_)),
        m_idle(static_cast<SM::StateID>(idle_)),
        m_idleDuration(idleDurationRange_),
        m_walk(static_cast<SM::StateID>(walk_)),
        m_walkDuration(walkDurationRange_)
    {}

    bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

protected:
    bool m_isWalking = false;
    FrameTimer<true> m_timer;

    SM::StateID m_idle;
    std::pair<uint32_t, uint32_t> m_idleDuration;

    SM::StateID m_walk;
    std::pair<uint32_t, uint32_t> m_walkDuration;
};

class BlindChaseState: public AIState
{
public:
    template<typename PLAYER_STATE_T>
    BlindChaseState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_,
    PLAYER_STATE_T idle_, PLAYER_STATE_T walk_, const unsigned int idleRange_) :
        AIState(stateId_, std::move(transitionableFrom_)),
        m_idle(static_cast<SM::StateID>(idle_)),
        m_walk(static_cast<SM::StateID>(walk_)),
        m_idleRange(idleRange_)
    {}

    void enter(EntityAnywhere owner_, SM::StateID from_) override;
    bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

protected:
    SM::StateID m_idle;
    SM::StateID m_walk;
    const unsigned int m_idleRange;
};

#if 0
class ProxySelectionState: public NodeState
{
public:
    template<typename PLAYER_STATE_T>
    ProxySelectionState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_,
    const std::vector<PLAYER_STATE_T> states_, std::vector<float> &&rangeLimits_, EntityAnywhere target_) :
        NodeState(stateId_, std::move(transitionableFrom_)),
        m_rangeLimits(std::move(rangeLimits_)),
        m_target(std::move(target_))
    {
        for (const auto &el : states_)
        {
            m_states.push_back(static_cast<SM::StateID>(el));
        }
    }

    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

protected:
    std::vector<SM::StateID> m_states;
    std::vector<float> m_rangeLimits;
    EntityAnywhere m_target;
};
#endif

class MoveTowards : public AIState
{
public:
    template<typename PLAYER_STATE_T>
    MoveTowards(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_,
        PLAYER_STATE_T walk_) :
        AIState(stateId_, std::move(transitionableFrom_)),
        m_walk(static_cast<SM::StateID>(walk_))
    {}

    void enter(EntityAnywhere owner_, SM::StateID from_) override;
    bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

private:
    SM::StateID m_walk;
};

class JumpTowards : public AIState
{
public:
    template<typename PLAYER_STATE_T>
    JumpTowards(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_,
        PLAYER_STATE_T prejump_) :
        AIState(stateId_, std::move(transitionableFrom_)),
        m_prejump(static_cast<SM::StateID>(prejump_))
    {}

    bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

private:
    SM::StateID m_prejump;
};

class NavigateGraphChase : public NodeState
{
public:
    template<typename PLAYER_STATE_T>
    NavigateGraphChase(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_,
        PLAYER_STATE_T moveTowards_, PLAYER_STATE_T jumpTowards, PLAYER_STATE_T noConnection_, PLAYER_STATE_T onSuccess_) :
        NodeState(stateId_, std::move(transitionableFrom_)),
        m_noConnection(static_cast<SM::StateID>(noConnection_)),
        m_moveTowards(static_cast<SM::StateID>(moveTowards_)),
        m_jumpTowards(static_cast<SM::StateID>(jumpTowards)),
        m_onSuccess(static_cast<SM::StateID>(onSuccess_))
    {}
    
    void enter(EntityAnywhere owner_, SM::StateID from_) override;
    bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

private:
    SM::StateID m_noConnection;
    SM::StateID m_moveTowards;
    SM::StateID m_jumpTowards;
    SM::StateID m_onSuccess;
};

#endif
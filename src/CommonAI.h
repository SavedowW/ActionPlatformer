#ifndef COMMON_AI_H_
#define COMMON_AI_H_
#include "StateMachine.h"

struct ComponentAI
{
    StateMachine m_sm;
    std::optional<CharState> m_requestedState;
    ORIENTATION m_requestedOrientation = ORIENTATION::UNSPECIFIED;
};

class AIState: public GenericState
{
public:
    template<typename PLAYER_STATE_T>
    AIState(PLAYER_STATE_T stateId_, const std::string &stateName_, StateMarker &&transitionableFrom_) :
        GenericState(stateId_, stateName_, std::move(transitionableFrom_))
    {}

    virtual void enter(EntityAnywhere owner_, CharState from_) override;

    AIState &setEnterRequestedState(std::optional<CharState> m_enterRequestedState_);
    AIState &setEnterRequestedOrientation(std::optional<ORIENTATION> enterRequestedOrientation_);

protected:
    std::optional<CharState> m_enterRequestedState;
    std::optional<ORIENTATION> m_enterRequestedOrientation;
};

class RandomRoamState: public NodeState
{
public:
    template<typename PLAYER_STATE_T>
    RandomRoamState(PLAYER_STATE_T stateId_, const std::string &stateName_, StateMarker &&transitionableFrom_,
    PLAYER_STATE_T idle_, PLAYER_STATE_T walk_, std::pair<uint32_t, uint32_t> idleDurationRange_, std::pair<uint32_t, uint32_t> walkDurationRange_) :
        NodeState(stateId_, stateName_, std::move(transitionableFrom_)),
        m_idle(static_cast<CharState>(idle_)),
        m_walk(static_cast<CharState>(walk_)),
        m_idleDuration(idleDurationRange_),
        m_walkDuration(walkDurationRange_)
    {}

    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

protected:
    bool m_isWalking = false;
    FrameTimer<true> m_timer;

    CharState m_idle;
    std::pair<uint32_t, uint32_t> m_idleDuration;

    CharState m_walk;
    std::pair<uint32_t, uint32_t> m_walkDuration;
};

class BlindChaseState: public AIState
{
public:
    template<typename PLAYER_STATE_T>
    BlindChaseState(PLAYER_STATE_T stateId_, const std::string &stateName_, StateMarker &&transitionableFrom_,
    PLAYER_STATE_T idle_, PLAYER_STATE_T walk_, EntityAnywhere target_, float idleRange_) :
        AIState(stateId_, stateName_, std::move(transitionableFrom_)),
        m_idle(static_cast<CharState>(idle_)),
        m_walk(static_cast<CharState>(walk_)),
        m_target(target_),
        m_idleRange(idleRange_)
    {}

    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

protected:
    CharState m_idle;
    CharState m_walk;
    EntityAnywhere m_target;
    float m_idleRange;
};

class ProxySelectionState: public NodeState
{
public:
    template<typename PLAYER_STATE_T>
    ProxySelectionState(PLAYER_STATE_T stateId_, const std::string &stateName_, StateMarker &&transitionableFrom_,
    const std::vector<PLAYER_STATE_T> states_, std::vector<float> &&rangeLimits_, EntityAnywhere target_) :
        NodeState(stateId_, stateName_, std::move(transitionableFrom_)),
        m_rangeLimits(std::move(rangeLimits_)),
        m_target(std::move(target_))
    {
        for (const auto &el : states_)
        {
            m_states.push_back(static_cast<CharState>(el));
        }
    }

    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;

protected:
    std::vector<CharState> m_states;
    std::vector<float> m_rangeLimits;
    EntityAnywhere m_target;
};

#endif
#ifndef STATE_MACHINE_HPP_
#define STATE_MACHINE_HPP_
#include "StateMachine.h"

template<typename PLAYER_STATE_T>
void StateMachine::switchCurrentState(EntityAnywhere owner_, PLAYER_STATE_T stateId_)
{
    if (static_cast<CharState>(stateId_) != std::numeric_limits<CharState>::max())
        switchCurrentState(owner_, m_states[m_stateIds[static_cast<CharState>(stateId_)]].get());
}

template<typename PLAYER_STATE_T>
void StateMachine::setInitialState(PLAYER_STATE_T state_)
{
    auto initstate = static_cast<CharState>(state_);
    m_framesInState = 0;
    m_currentState = m_states[m_stateIds[initstate]].get();
}

template<typename PLAYER_STATE_T>
GenericState &GenericState::setOutdatedTransition(PLAYER_STATE_T state_, uint32_t duration_)
{
    m_transitionOnOutdated = static_cast<CharState>(state_);
    m_duration = duration_;
    return *this;
}

template<typename PLAYER_STATE_T>
PhysicalState::PhysicalState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_, ResID anim_) :
    GenericState(stateId_, std::move(transitionableFrom_)),
    m_anim(anim_),
    m_appliedInertiaMultiplier(Vector2{1.0f, 1.0f}),
    m_drag(Vector2{1.0f, 0.0f}),
    m_transitionVelocityMultiplier(Vector2{1.0f, 1.0f}),
    m_canFallThrough(true)
{}

template<typename PLAYER_STATE_T>
inline PhysicalState &PhysicalState::setTransitionOnLostGround(PLAYER_STATE_T state_)
{
    m_transitionOnLostGround = static_cast<CharState>(state_);
    return *this;
}

template<typename PLAYER_STATE_T>
PhysicalState &PhysicalState::addTransitionAnim(PLAYER_STATE_T oldState_, ResID anim_)
{
    m_uniqueTransitionAnims[static_cast<CharState>(oldState_)] = anim_;
    return *this;
}

template<typename PLAYER_STATE_T>
NodeState::NodeState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_) :
    GenericState(stateId_, std::move(transitionableFrom_))
{}

#endif

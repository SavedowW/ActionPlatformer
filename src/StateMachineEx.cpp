#include "StateMachineEx.h"

namespace SM
{
    StatePossessor::StatePossessor(MachineID stateMachine_, StateID currentState_) :
        m_stateMachine{stateMachine_},
        m_currentState{currentState_}
    {}

    CompoundState::CompoundState(const StateID &id_, std::string name_) noexcept :
        m_id{id_},
        m_name{std::move(name_)}
    {}

    void CompoundState::addToBody(std::unique_ptr<BodyCallable> &&bodyCallable_)
    {
        m_body.emplace_back(std::move(bodyCallable_));
    }
} // SM

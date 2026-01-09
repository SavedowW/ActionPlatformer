#ifndef STATE_MACHINE_EX_H_
#define STATE_MACHINE_EX_H_
#include "Core/ECS/ComponentExtractor.h"
#include "Core/Timer.h"
#include <string>
#include <memory>
#include <unordered_map>



// State Machine
namespace SM
{
    using MachineID = size_t;
    using StateID = size_t;

    /**
     * ECS Component for everyone related to the state machine
     */
    class StatePossessor
    {
    public:
        StatePossessor(MachineID stateMachine_, StateID currentState_);

        StatePossessor(StatePossessor&&) noexcept = default;
        StatePossessor(const StatePossessor&) noexcept = default;

        MachineID machineId() const { return m_stateMachine; };
        StateID stateId() const { return m_currentState; };
        uint32_t timeInState() const { return m_framesInState; };

    private:
        MachineID m_stateMachine;
        StateID m_currentState;
        uint32_t m_framesInState = 0;
    };

    class BodyCallable
    {
    public:
        virtual void operator()(const ComponentViewer &viewer_, uint32_t framesInState_) const = 0;
        virtual ~BodyCallable() = default;
    };

    class CompoundState
    {
    public:
        CompoundState(const StateID &id_, std::string name_) noexcept;
        void addToBody(std::unique_ptr<BodyCallable> &&bodyCallable_);

    private:
        const StateID m_id;
        const std::string m_name;

        std::vector<std::unique_ptr<BodyCallable>> m_body;
    };

    /**
     *  A new shiny flexible state machine:
     *   - Dynamically defined through the external file (mostly because templates are painful)
     *   - Fully immutable, shared between it's users
     *   - Instance's state is stored entirely in the component or components
     *   - Closer to a state chart - all states exist at the same level and are connected through "pipes"
     *   - Plug & Play - as long as each state knows what and how can it transition into, it can be freely inserted into the SM
     */
    class StateMachine
    {
    public:
        StateMachine() = default;
        StateMachine(const StateMachine&) = delete;
        StateMachine &operator=(const StateMachine&) = delete;
        StateMachine(StateMachine&&) noexcept = default;
        StateMachine &operator=(StateMachine&&) noexcept = default;

    private:
        std::unordered_map<StateID, std::unique_ptr<CompoundState>> m_states;
    };

}

#endif

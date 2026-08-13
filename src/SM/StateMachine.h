#pragma once
#include "Core/Timer.h"
#include "Core/Vector2.hpp"
#include "Core/NavGraph.h"
#include "entt/entity/fwd.hpp"
#include "Core/ECS/ComponentsView.h"
#include <memory>
#include <unordered_map>

enum class TraverseTraits : Traverse::TraitT {
    WALK = 0b001,
    JUMP = 0b010,
    FALL = 0b100
};

// State Machine
namespace SM
{
    using StateID = uint8_t;

    template<typename StateIDT>
    struct TransitionData
    {
        StateIDT fromState;
        StateIDT intoState;

        // UNSPECIFIED if there is no transition
        Flag<Orientation> intoOrientation;

        TransitionData(StateIDT fromState_, StateIDT intoState_, Flag<Orientation> intoOrientation_) noexcept;
        TransitionData(const TransitionData&) noexcept = default;
        TransitionData(TransitionData&&) noexcept = default;
        TransitionData &operator=(const TransitionData&) noexcept = default;
        TransitionData &operator=(TransitionData&&) noexcept = default;
    };

    /**
     * ECS Component for everyone related to the state machine
     */
    template<typename StateIDT>
    class StatePossessor
    {
    public:
        StatePossessor(StateIDT initialState_);

        StatePossessor(StatePossessor&&) noexcept = default;
        StatePossessor(const StatePossessor&) noexcept = default;

        StateIDT stateId() const noexcept { return m_currentState; };
        uint32_t framesInState() const noexcept { return m_framesInState; };
        void addFrameInState() noexcept { m_framesInState++; };
        void resetFramesInState() noexcept { m_framesInState = 0; };
        void setState(StateIDT newState_) noexcept;

    private:
        StateIDT m_currentState;
        uint32_t m_framesInState = 0;
    };

    template<typename StateIDT, IsComponentsView ViewT>
    class GenericState
    {
    public:
        GenericState(const StateIDT &id_) noexcept;
        StateIDT id() const noexcept { return m_id; };
        void update(const ViewT &view_) const;
        virtual TransitionData<StateIDT> canTransition(const ViewT &view_) const = 0;
        void handleTransitionFrom(const ViewT &view_, const TransitionData<StateIDT>& transition_) const;
        void handleTransitionInto(const ViewT &view_, const TransitionData<StateIDT>& transition_) const;

        virtual ~GenericState() = default;

    protected:
        virtual void updateImpl(const ViewT &view_) const = 0;
        virtual void handleTransitionFromImpl(const ViewT &view_, const TransitionData<StateIDT>& transition_) const = 0;
        virtual void handleTransitionIntoImpl(const ViewT &view_, const TransitionData<StateIDT>& transition_) const = 0;

        const StateIDT m_id;
    };

    /**
     *  A new shiny flexible state machine. Completely immutable, stored as a single instance in the respective system
     */
    template<typename StateIDT, IsComponentsView ViewT>
    class StateMachine
    {
    public:
        StateMachine() = default;
        StateMachine(const StateMachine&) = delete;
        StateMachine &operator=(const StateMachine&) = delete;
        StateMachine(StateMachine&&) noexcept = default;
        StateMachine &operator=(StateMachine&&) noexcept = default;

        void addState(std::unique_ptr<GenericState<StateIDT, ViewT>> &&newState_);

        void init(entt::registry &reg_, entt::entity idx_);
        void update(entt::registry &reg_) const;

    private:
        std::unordered_map<StateIDT, std::unique_ptr<GenericState<StateIDT, ViewT>>> m_states;
    };

}


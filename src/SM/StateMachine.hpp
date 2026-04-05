#pragma once
#include "Core/CoreComponents.h"
#include "Core/Vector2.hpp"
#include "StateMachine.h"
#include <entt/entt.hpp>

namespace SM
{
    template<typename StateIDT>
    TransitionData<StateIDT>::TransitionData(StateIDT fromState_, StateIDT intoState_, ORIENTATION intoOrientation_) noexcept :
        fromState{fromState_},
        intoState{intoState_},
        intoOrientation{intoOrientation_}
    {}

    template<typename StateIDT>
    StatePossessor<StateIDT>::StatePossessor(StateIDT initialState_) :
        m_currentState{initialState_}
    {}

    template<typename StateIDT>
    void StatePossessor<StateIDT>::setState(StateIDT newState_) noexcept
    {
        m_currentState = newState_;
        m_framesInState = 0;
    }

    template<typename StateIDT, typename ViewT>
    GenericState<StateIDT, ViewT>::GenericState(const StateIDT &id_) noexcept :
        m_id{id_}
    {}

    template<typename StateIDT, typename ViewT>
    void GenericState<StateIDT, ViewT>::update(const ViewT &view_) const
    {
        updateImpl(view_);
        view_.template get<StatePossessor<StateIDT>>().addFrameInState();
    }

    template<typename StateIDT, typename ViewT>
    void GenericState<StateIDT, ViewT>::handleTransitionFrom(const ViewT &view_, const TransitionData<StateIDT>& transition_) const
    {
        handleTransitionFromImpl(view_, transition_);
    }

    template<typename StateIDT, typename ViewT>
    void GenericState<StateIDT, ViewT>::handleTransitionInto(const ViewT &view_, const TransitionData<StateIDT>& transition_) const
    {
        handleTransitionIntoImpl(view_, transition_);
        view_.template get<StatePossessor<StateIDT>>().setState(m_id);
    }

    template<typename StateIDT, typename ViewT>
    void StateMachine<StateIDT, ViewT>::addState(std::unique_ptr<GenericState<StateIDT, ViewT>> &&newState_)
    {
        m_states[newState_->id()] = std::move(newState_);
    }

    template<typename StateIDT, typename ViewT>
    void StateMachine<StateIDT, ViewT>::init(entt::registry &reg_, entt::entity idx_)
    {
        const auto &possessor = reg_.get<StatePossessor<StateIDT>>(idx_);
        const auto &transform = reg_.get<ComponentTransform>(idx_);
        const GenericState<StateIDT, ViewT> &state = *m_states.at(possessor.stateId());

        const auto refs = ViewT::makeRefs(reg_, idx_);
        ViewT view{refs};
        state.handleTransitionInto(view, TransitionData{possessor.stateId(), possessor.stateId(), transform.m_orientation});
    }

    template<typename StateIDT, typename ViewT>
    void StateMachine<StateIDT, ViewT>::update(entt::registry &reg_) const
    {
        auto view = ViewT::makeView(reg_);
        for (const auto &ent : view.each())
        {
            ViewT entityView{ent};
            const StatePossessor<StateIDT> &possessor = entityView.template cget<StatePossessor<StateIDT>>();
            const GenericState<StateIDT, ViewT> &state = *m_states.at(possessor.stateId());
            state.update(entityView);
            
            TransitionData transition = state.canTransition(entityView);

            while (transition.intoOrientation != ORIENTATION::UNSPECIFIED)
            {
                auto &newState = *m_states.at(transition.intoState);
                state.handleTransitionFrom(entityView, transition);
                newState.handleTransitionInto(entityView, transition);
                transition = newState.canTransition(entityView);
            }
        }
    }
} // SM

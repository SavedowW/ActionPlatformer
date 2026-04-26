#pragma once
#include "CompoundState.h"
#include "StateMachine.hpp"
#include <memory>

template<typename... Args>
template<typename... CallablesT>
AbstractCallable<Args...>::Compound<CallablesT...>::Compound(CallablesT&&... callables_) :
    m_callables(std::forward<CallablesT>(callables_)...)
{}

template<typename... Args>
template<typename... CallablesT>
void AbstractCallable<Args...>::Compound<CallablesT...>::operator()(Args&&... args_) const
{
    std::apply([&](auto&&... callable) {((callable(std::forward<Args>(args_)...)), ...);}, m_callables);
}


template<typename... Args>
template<typename... CallablesT>
auto AbstractCallable<Args...>::makeUniqueCompound(CallablesT&&... callables_)
{
    return std::make_unique<AbstractCallable<Args...>::Compound<CallablesT...>>(std::forward<CallablesT>(callables_)...);
}

namespace SM
{
    template<typename... CallablesT>
    CallBatch<CallablesT...>::CallBatch(CallablesT&&... callables_) :
        m_callables(std::forward<CallablesT>(callables_)...)
    {}

    template<typename... CallablesT>
    template<typename... Args>
    void CallBatch<CallablesT...>::operator()(Args&&... args_) const
    {
        std::apply([&](auto&&... callable) {((callable(std::forward<Args>(args_)...)), ...);}, m_callables);
    }

    template<typename StateIDT, typename ViewT, typename UpdaterT, typename TransCheckT, typename HandlerFromT, typename HandlerIntoT>
    CompoundState<StateIDT, ViewT, UpdaterT, TransCheckT, HandlerFromT, HandlerIntoT>
        ::CompoundState(const StateIDT &id_, UpdaterT &&updater_, TransCheckT &&transCheck_, HandlerFromT &&handlerFrom_, HandlerIntoT &&handlerTo_) :
        GenericState<StateIDT, ViewT>{id_},
        m_updater{std::move(updater_)},
        m_transitionCheck{std::move(transCheck_)},
        m_transitionFromHandler{std::move(handlerFrom_)},
        m_transitionIntoHandler{std::move(handlerTo_)}
    {

    }

    template<typename StateIDT, typename ViewT, typename UpdaterT, typename TransCheckT, typename HandlerFromT, typename HandlerIntoT>
    void CompoundState<StateIDT, ViewT, UpdaterT, TransCheckT, HandlerFromT, HandlerIntoT>::updateImpl(const ViewT &view_) const
    {
        m_updater(view_);
    }

    template<typename StateIDT, typename ViewT, typename UpdaterT, typename TransCheckT, typename HandlerFromT, typename HandlerIntoT>
    TransitionData<StateIDT> CompoundState<StateIDT, ViewT, UpdaterT, TransCheckT, HandlerFromT, HandlerIntoT>::canTransition(const ViewT &view_) const
    {
        auto res = m_transitionCheck(view_);
        res.fromState = this->m_id;
        return res;
    }

    template<typename StateIDT, typename ViewT, typename UpdaterT, typename TransCheckT, typename HandlerFromT, typename HandlerIntoT>
    void CompoundState<StateIDT, ViewT, UpdaterT, TransCheckT, HandlerFromT, HandlerIntoT>::handleTransitionFromImpl(const ViewT &view_, const TransitionData<StateIDT>& transition_) const
    {
        return m_transitionFromHandler(view_, transition_);
    }

    template<typename StateIDT, typename ViewT, typename UpdaterT, typename TransCheckT, typename HandlerFromT, typename HandlerIntoT>
    void CompoundState<StateIDT, ViewT, UpdaterT, TransCheckT, HandlerFromT, HandlerIntoT>::handleTransitionIntoImpl(const ViewT &view_, const TransitionData<StateIDT>& transition_) const
    {
        return m_transitionIntoHandler(view_, transition_);
    }
    

    template<typename StateIDT, typename ViewT>
    RulePipesContainer<StateIDT, ViewT> &RulePipesContainer<StateIDT, ViewT>::setPipe(const StateIDT &id_, std::unique_ptr<AbstractCallable<const ViewT&, const TransitionData<StateIDT>&>> &&pipe_)
    {
        m_pipes[id_] = std::move(pipe_);
        return *this;
    }

    template<typename StateIDT, typename ViewT>
    RulePipesContainer<StateIDT, ViewT> &RulePipesContainer<StateIDT, ViewT>::setDefaultPipe(std::unique_ptr<AbstractCallable<const ViewT&, const TransitionData<StateIDT>&>> &&pipe_)
    {
        m_defaultPipe = std::move(pipe_);
        return *this;
    }

    template<typename StateIDT, typename ViewT>
    void RulePipesContainer<StateIDT, ViewT>::operator()(const ViewT &view_, const TransitionData<StateIDT> &transition_) const
    {
        auto found = m_pipes.find(transition_.fromState);
        if (found != m_pipes.end())
            (*found->second)(view_, transition_);
        else
            (*m_defaultPipe)(view_, transition_);
    }

} // SM

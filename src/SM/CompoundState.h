#pragma once
#include "StateMachine.h"

/**
 * Lets you tie a bunch of optional callables together and call them with specified arguments.
 * AbstractCallable::Compound is an implementation that ties a bunch of callables together and calls them under arguments known from interface.
 * No return value since then there is no common logic
 */
template<typename... Args>
class AbstractCallable
{
public:
    virtual void operator()(Args&&... args_) const = 0;
    virtual ~AbstractCallable() = default;    

    template<typename... Callables>
    class Compound;

    template<typename... CallablesT>
    static auto makeUniqueCompound(CallablesT&&... callables_);
};

template<typename... Args>
template<typename... CallablesT>
class AbstractCallable<Args...>::Compound : public AbstractCallable<Args...>
{
public:
    Compound(CallablesT&&... callables_);
    Compound(const Compound &) noexcept = default;
    Compound(Compound &&) noexcept = default;
    Compound &operator=(const Compound &) noexcept = default;
    Compound &operator=(Compound &&) noexcept = default;

    void operator()(Args&&... args_) const override;

private:
    std::tuple<CallablesT...> m_callables;
};

namespace SM
{
    /**
     * Lets you tie a bunch of callables together and call them with any arguments.
     * No interface, everything is entirely static
     */
    template<typename... CallablesT>
    class CallBatch
    {
    public:
        CallBatch(CallablesT&&... callables_);
        CallBatch(const CallBatch &) noexcept = default;
        CallBatch(CallBatch &&) noexcept = default;
        CallBatch &operator=(const CallBatch &) noexcept = default;
        CallBatch &operator=(CallBatch &&) noexcept = default;

        template<typename... Args>
        void operator()(Args&&... args_) const;


    private:
        std::tuple<CallablesT...> m_callables;
    };


    template<typename StateIDT, typename ViewT>
    class RulePipesContainer
    {
    public:
        RulePipesContainer &setPipe(const StateIDT &id_, std::unique_ptr<AbstractCallable<const ViewT&, const TransitionData<StateIDT>&>> &&pipe_);
        RulePipesContainer &setDefaultPipe(std::unique_ptr<AbstractCallable<const ViewT&, const TransitionData<StateIDT>&>> &&pipe_);

        void operator()(const ViewT&, const TransitionData<StateIDT>&) const;
    
    private:
        std::unordered_map<StateIDT, std::unique_ptr<AbstractCallable<const ViewT&, const TransitionData<StateIDT>&>>> m_pipes;
        std::unique_ptr<AbstractCallable<const ViewT&, const TransitionData<StateIDT>&>> m_defaultPipe;
    };

    template<typename StateIDT, typename ViewT, typename UpdaterT, typename TransCheckT, typename HandlerFromT, typename HandlerIntoT>
    class CompoundState : public GenericState<StateIDT, ViewT>
    {
    public:
        CompoundState(const StateIDT &id_, UpdaterT &&updater_, TransCheckT &&transCheck_, HandlerFromT &&handlerFrom_, HandlerIntoT &&handlerTo_);
        TransitionData<StateIDT> canTransition(const ViewT &view_) const override;

    protected:
        void updateImpl(const ViewT &view_) const override;
        void handleTransitionFromImpl(const ViewT &view_, const TransitionData<StateIDT>& transition_) const override;
        void handleTransitionIntoImpl(const ViewT &view_, const TransitionData<StateIDT>& transition_) const override;

        const UpdaterT m_updater;
        const TransCheckT m_transitionCheck;
        const HandlerFromT m_transitionFromHandler;
        const HandlerIntoT m_transitionIntoHandler;
    };
} // SM

#pragma once
#include "CompoundState.h"
#include "TransitionCheck.h"

namespace SM
{

template<typename StateIDT, typename ViewT>
class Make
{
public:
    template<typename... Args>
    static std::unique_ptr<CompoundState<StateIDT, ViewT, Args...>> state(StateIDT id_, Args&&... args_);

    class RulePipe
    {
    public:
        SM::RulePipesContainer<StateIDT, ViewT> &&done();

        template<typename... CallablesT>
        RulePipe &setPipe(const StateIDT &state_, CallablesT&&... callables_);

        template<typename... CallablesT>
        RulePipe &setDefaultPipe(CallablesT&&... callables_);

    private:
        SM::RulePipesContainer<StateIDT, ViewT> m_container;
    };

    class SequentialConditions
    {
    public:
        Sequential<StateIDT, ViewT> &&done();
        SequentialConditions &addCondition(std::unique_ptr<AbstractCondition<StateIDT, ViewT>> &&condition_);

    private:
        Sequential<StateIDT, ViewT> m_conditions;
    };
};

} // SM

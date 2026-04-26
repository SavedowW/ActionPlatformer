#pragma once
#include "CompoundState.h"
#include "TransitionChecks.h"

namespace SM
{

template<typename StateIDT, typename ViewT>
class Make
{
public:
    template<typename... Args>
    static std::unique_ptr<SM::CompoundState<StateIDT, ViewT, Args...>> state(StateIDT id_, Args&&... args_);

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
        TransitionChecks<StateIDT, ViewT>::Base::Sequential &&done();
        SequentialConditions &addCondition(std::unique_ptr<typename TransitionChecks<StateIDT, ViewT>::Base::AbstractCondition> &&condition_);

    private:
        TransitionChecks<StateIDT, ViewT>::Base::Sequential m_conditions;
    };
};

} // SM

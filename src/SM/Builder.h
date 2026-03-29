#pragma once
#include "CompoundState.h"

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
        SM::RulePipesContainer<StateIDT, ViewT> _container;
    };
};

} // SM

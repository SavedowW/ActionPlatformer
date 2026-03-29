#pragma once
#include "Builder.h"

namespace SM
{

    template<typename StateIDT, typename ViewT>
    template<typename... Args>
    std::unique_ptr<CompoundState<StateIDT, ViewT, Args...>> Make<StateIDT, ViewT>::state(StateIDT id_, Args&&... args_)
    {
        return std::make_unique<CompoundState<StateIDT, ViewT, Args...>>(id_, std::forward<Args>(args_)...);
    }

    template<typename StateIDT, typename ViewT>
    SM::RulePipesContainer<StateIDT, ViewT> &&Make<StateIDT, ViewT>::RulePipe::done()
    {
        return std::move(_container);
    }

    template<typename StateIDT, typename ViewT>
    template<typename... CallablesT>
    Make<StateIDT, ViewT>::RulePipe &Make<StateIDT, ViewT>::RulePipe::setPipe(const StateIDT &state_, CallablesT&&... callables_)
    {
        _container.setPipe(state_, AbstractCallable<const ViewT&, const SM::TransitionData<StateIDT>&>::makeUniqueCompound(std::forward<CallablesT>(callables_)...));
        return *this;
    }

    template<typename StateIDT, typename ViewT>
    template<typename... CallablesT>
    Make<StateIDT, ViewT>::RulePipe &Make<StateIDT, ViewT>::RulePipe::setDefaultPipe(CallablesT&&... callables_)
    {
        _container.setDefaultPipe(AbstractCallable<const ViewT&, const SM::TransitionData<StateIDT>&>::makeUniqueCompound(std::forward<CallablesT>(callables_)...));
        return *this;
    }

}

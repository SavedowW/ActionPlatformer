#pragma once
#include "SM/Builder.h"
#include "SM/CompoundState.hpp"
#include "SM/TransitionChecks.hpp"

namespace SM
{

    template<typename StateIDT, typename ViewT>
    template<typename... Args>
    std::unique_ptr<SM::CompoundState<StateIDT, ViewT, Args...>> Make<StateIDT, ViewT>::state(StateIDT id_, Args&&... args_)
    {
        return std::make_unique<SM::CompoundState<StateIDT, ViewT, Args...>>(id_, std::forward<Args>(args_)...);
    }

    template<typename StateIDT, typename ViewT>
    SM::RulePipesContainer<StateIDT, ViewT> &&Make<StateIDT, ViewT>::RulePipe::done()
    {
        return std::move(m_container);
    }

    template<typename StateIDT, typename ViewT>
    template<typename... CallablesT>
    Make<StateIDT, ViewT>::RulePipe &Make<StateIDT, ViewT>::RulePipe::setPipe(const StateIDT &state_, CallablesT&&... callables_)
    {
        m_container.setPipe(state_, AbstractCallable<const ViewT&, const SM::TransitionData<StateIDT>&>::makeUniqueCompound(std::forward<CallablesT>(callables_)...));
        return *this;
    }

    template<typename StateIDT, typename ViewT>
    template<typename... CallablesT>
    Make<StateIDT, ViewT>::RulePipe &Make<StateIDT, ViewT>::RulePipe::setDefaultPipe(CallablesT&&... callables_)
    {
        m_container.setDefaultPipe(AbstractCallable<const ViewT&, const SM::TransitionData<StateIDT>&>::makeUniqueCompound(std::forward<CallablesT>(callables_)...));
        return *this;
    }


    template<typename StateIDT, typename ViewT>
    typename TransitionChecks<StateIDT, ViewT>::Base::Sequential &&Make<StateIDT, ViewT>::SequentialConditions::done()
    {
        return std::move(m_conditions);
    }

    template<typename StateIDT, typename ViewT>
    Make<StateIDT, ViewT>::SequentialConditions &Make<StateIDT, ViewT>::SequentialConditions::addCondition(std::unique_ptr<typename TransitionChecks<StateIDT, ViewT>::Base::AbstractCondition> &&condition_)
    {
        m_conditions.addCondition(std::move(condition_));
        return *this;
    }

}

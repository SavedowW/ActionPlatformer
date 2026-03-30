#pragma once
#include "Builder.h"
#include "SM/CompoundState.hpp"
#include "SM/TransitionCheck.hpp"

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
    Sequential<StateIDT, ViewT> &&Make<StateIDT, ViewT>::SequentialConditions::done()
    {
        return std::move(m_conditions);
    }

    template<typename StateIDT, typename ViewT>
    Make<StateIDT, ViewT>::SequentialConditions &Make<StateIDT, ViewT>::SequentialConditions::addCondition(std::unique_ptr<AbstractCondition<StateIDT, ViewT>> &&condition_)
    {
        m_conditions.addCondition(std::move(condition_));
        return *this;
    }

}

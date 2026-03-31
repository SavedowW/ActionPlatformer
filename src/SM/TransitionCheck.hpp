#pragma once
#include "Core/CoreComponents.h"
#include "StateMachine.h"
#include "TransitionCheck.h"
#include "StateMachine.hpp"

namespace SM
{
    template<typename StateIDT, typename ViewT>
    AbstractCondition<StateIDT, ViewT>::AbstractCondition(const StateIDT &state_) :
        m_state{state_}
    {}

    template<typename StateIDT, typename ViewT>
    AbstractCondition<StateIDT, ViewT>::operator StateIDT() const
    {
        return m_state;
    }

    template<typename StateIDT, typename ViewT>
    TransitionData<StateIDT> Sequential<StateIDT, ViewT>::operator()(const ViewT &view_) const
    {
        for (const auto &con : m_conditions)
        {
            const auto res = (*con)(view_);
            if (res != ORIENTATION::UNSPECIFIED)
            {
                const auto state = StateIDT(*con);
                return TransitionData<StateIDT>{state, state, res};
            }
        }

        return TransitionData<StateIDT>{static_cast<StateIDT>(0), static_cast<StateIDT>(0), ORIENTATION::UNSPECIFIED};
    }

    template<typename StateIDT, typename ViewT>
    Sequential<StateIDT, ViewT> &Sequential<StateIDT, ViewT>::addCondition(std::unique_ptr<AbstractCondition<StateIDT, ViewT>> &&condition_)
    {
        m_conditions.emplace_back(std::move(condition_));
        return *this;
    }


    namespace Transition::Checks
    {
        template<typename StateIDT, typename ViewT>
        OnPhysEvent<StateIDT, ViewT>::OnPhysEvent(const StateIDT &state_, const PhysicalEvents::Events &event_) :
            AbstractCondition<StateIDT, ViewT>(state_),
            m_event{event_}
        {}

        template<typename StateIDT, typename ViewT>
        ORIENTATION OnPhysEvent<StateIDT, ViewT>::operator()(const ViewT &view_)
        {
            if (view_.template cget<PhysicalEvents>().checkEvent(m_event))
                return view_.template cget<ComponentTransform>().m_orientation;

            return ORIENTATION::UNSPECIFIED;
        }

        template<typename StateIDT, typename ViewT>
        OnTimer<StateIDT, ViewT>::OnTimer(const StateIDT &state_, uint32_t framesLimit_) :
            AbstractCondition<StateIDT, ViewT>(state_),
            m_framesLimit{framesLimit_}
        {}

        template<typename StateIDT, typename ViewT>
        ORIENTATION OnTimer<StateIDT, ViewT>::operator()(const ViewT &view_)
        {
            if (view_.template cget<StatePossessor<StateIDT>>().framesInState() >= m_framesLimit)
                return view_.template cget<ComponentTransform>().m_orientation;

            return ORIENTATION::UNSPECIFIED;
        }
    }
}

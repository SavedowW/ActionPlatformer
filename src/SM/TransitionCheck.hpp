#pragma once
#include "Core/CoreComponents.h"
#include "Core/InputResolver.h"
#include "Core/Vector2.hpp"
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
        OnPhysEvent<StateIDT, ViewT>::OnPhysEvent(const StateIDT &state_, const PhysicalEvents::Events &event_, bool isSet_) :
            AbstractCondition<StateIDT, ViewT>(state_),
            m_event{event_},
            m_isSet{isSet_}
        {}

        template<typename StateIDT, typename ViewT>
        ORIENTATION OnPhysEvent<StateIDT, ViewT>::operator()(const ViewT &view_)
        {
            if (view_.template cget<PhysicalEvents>().checkEvent(m_event) == m_isSet)
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


        template<typename StateIDT, typename ViewT>
        InputTest<StateIDT, ViewT>::InputTest(const StateIDT &state_, InputMotions input_, Flag<OrientationOptions> options_) :
            AbstractCondition<StateIDT, ViewT>(state_),
            m_input{input_},
            m_orientations(options_)
        {}

        template<typename StateIDT, typename ViewT>
        ORIENTATION InputTest<StateIDT, ViewT>::operator()(const ViewT &view_)
        {
            const auto &transform = view_.template cget<ComponentTransform>();
            const auto &inputs = view_.template cget<InputResolver>();

            if (transform.m_orientation == ORIENTATION::RIGHT)
            {
                if ((m_orientations & OrientationOptions::RIGHT) == OrientationOptions::RIGHT || (m_orientations & OrientationOptions::SAME) == OrientationOptions::SAME)
                {
                    if (inputs.checkInput(m_input, ORIENTATION::RIGHT, 0))
                        return ORIENTATION::RIGHT;
                }

                if ((m_orientations & OrientationOptions::LEFT) == OrientationOptions::LEFT || (m_orientations & OrientationOptions::OPPOSITE) == OrientationOptions::OPPOSITE)
                {
                    if (inputs.checkInput(m_input, ORIENTATION::LEFT, 0))
                        return ORIENTATION::LEFT;
                }
            }
            else
            {
                if ((m_orientations & OrientationOptions::LEFT) == OrientationOptions::LEFT || (m_orientations & OrientationOptions::SAME) == OrientationOptions::SAME)
                {
                    if (inputs.checkInput(m_input, ORIENTATION::LEFT, 0))
                        return ORIENTATION::LEFT;
                }

                if ((m_orientations & OrientationOptions::RIGHT) == OrientationOptions::RIGHT || (m_orientations & OrientationOptions::OPPOSITE) == OrientationOptions::OPPOSITE)
                {
                    if (inputs.checkInput(m_input, ORIENTATION::RIGHT, 0))
                        return ORIENTATION::RIGHT;
                }
            }

            return ORIENTATION::UNSPECIFIED;
        }
    }
}

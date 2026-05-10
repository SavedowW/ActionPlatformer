#pragma once
#include "Core/CoreComponents.h"
#include "Core/InputResolver.h"
#include "Core/Vector2.hpp"
#include "StateMachine.h"
#include "TransitionChecks.h"
#include "StateMachine.hpp"

template<typename StateIDT, typename ViewT>
SM::TransitionData<StateIDT> TransitionChecks<StateIDT, ViewT>::Base::Sequential::operator()(const ViewT &view_) const
{
    for (const auto &con : m_conditions)
    {
        const auto res = (*con)(view_);
        if (res != ORIENTATION::UNSPECIFIED)
        {
            const auto state = StateIDT(*con);
            return SM::TransitionData<StateIDT>{state, state, res};
        }
    }

    return SM::TransitionData<StateIDT>{static_cast<StateIDT>(0), static_cast<StateIDT>(0), ORIENTATION::UNSPECIFIED};
}

template<typename StateIDT, typename ViewT>
TransitionChecks<StateIDT, ViewT>::Base::Sequential &TransitionChecks<StateIDT, ViewT>::Base::Sequential::addCondition(std::unique_ptr<AbstractCondition> &&condition_)
{
    m_conditions.emplace_back(std::move(condition_));
    return *this;
}


template<typename StateIDT, typename ViewT>
TransitionChecks<StateIDT, ViewT>::Base::AbstractStateCondition::AbstractStateCondition(StateIDT state_) :
    m_state{state_}
{}

template<typename StateIDT, typename ViewT>
TransitionChecks<StateIDT, ViewT>::Base::AbstractStateCondition::operator StateIDT() const
{
    return m_state;
}


template<typename StateIDT, typename ViewT>
template<typename T>
TransitionChecks<StateIDT, ViewT>::Base::SinceFrameImpl<T>::SinceFrameImpl(uint32_t sinceFrame_, T &&condition_) :
    m_sinceFrame{sinceFrame_},
    m_condition{std::move(condition_)}
{}

template<typename StateIDT, typename ViewT>
template<typename T>
TransitionChecks<StateIDT, ViewT>::Base::SinceFrameImpl<T>::operator StateIDT() const
{
    return StateIDT(m_condition);
}

template<typename StateIDT, typename ViewT>
template<typename T>
ORIENTATION TransitionChecks<StateIDT, ViewT>::Base::SinceFrameImpl<T>::operator()(const ViewT &view_)
{
    const auto &stateDescr = view_.template cget<SM::StatePossessor<StateIDT>>();
    if (stateDescr.framesInState() >= m_sinceFrame)
        return m_condition(view_);

    return ORIENTATION::UNSPECIFIED;
}

template<typename StateIDT, typename ViewT>
template<typename T>
auto TransitionChecks<StateIDT, ViewT>::sinceFrame(uint32_t sinceFrame_, T &&condition_)
{
    return std::make_unique<typename TransitionChecks<StateIDT, ViewT>::Base::template SinceFrameImpl<T>>(sinceFrame_, std::forward<T>(condition_));
}



template<typename StateIDT, typename ViewT>
TransitionChecks<StateIDT, ViewT>::OnPhysEvent::OnPhysEvent(const StateIDT &state_, const PhysicalEvents::Events &event_, bool isSet_) :
    Base::AbstractStateCondition(state_),
    m_event{event_},
    m_isSet{isSet_}
{}

template<typename StateIDT, typename ViewT>
ORIENTATION TransitionChecks<StateIDT, ViewT>::OnPhysEvent::operator()(const ViewT &view_)
{
    if (view_.template cget<PhysicalEvents>().checkEvent(m_event) == m_isSet)
        return view_.template cget<ComponentTransform>().m_orientation;

    return ORIENTATION::UNSPECIFIED;
}

template<typename StateIDT, typename ViewT>
TransitionChecks<StateIDT, ViewT>::OnGrounded::OnGrounded(const StateIDT &state_, bool isGrounded_) :
    Base::AbstractStateCondition(state_),
    m_isGrounded{isGrounded_}
{}

template<typename StateIDT, typename ViewT>
ORIENTATION TransitionChecks<StateIDT, ViewT>::OnGrounded::operator()(const ViewT &view_)
{
    const auto &phys = view_.template cget<ComponentPhysical>();

    if ((phys.m_onGround != entt::null) == m_isGrounded)
        return view_.template cget<ComponentTransform>().m_orientation;

    return ORIENTATION::UNSPECIFIED;
}

template<typename StateIDT, typename ViewT>
TransitionChecks<StateIDT, ViewT>::OnTimer::OnTimer(const StateIDT &state_, uint32_t framesLimit_) :
    Base::AbstractStateCondition(state_),
    m_framesLimit{framesLimit_}
{}

template<typename StateIDT, typename ViewT>
ORIENTATION TransitionChecks<StateIDT, ViewT>::OnTimer::operator()(const ViewT &view_)
{
    if (view_.template cget<SM::StatePossessor<StateIDT>>().framesInState() >= m_framesLimit)
        return view_.template cget<ComponentTransform>().m_orientation;

    return ORIENTATION::UNSPECIFIED;
}


template<typename StateIDT, typename ViewT>
TransitionChecks<StateIDT, ViewT>::InputTest::InputTest(const StateIDT &state_, InputMotions input_, Flag<OrientationOptions> options_) :
    Base::AbstractStateCondition(state_),
    m_input{input_},
    m_orientations(options_)
{}

template<typename StateIDT, typename ViewT>
ORIENTATION TransitionChecks<StateIDT, ViewT>::InputTest::operator()(const ViewT &view_)
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

#include "Core/CoreComponents.h"
#include "StateMachine.hpp"
#include "PhysicalState.h"

namespace SM::Updaters
{
    template<typename StateIDT, typename ViewT>
    void Notify<StateIDT, ViewT>::operator()(const ViewT &view_) const
    {
        const StatePossessor<StateIDT> &owner = view_.template cget<StatePossessor<StateIDT>>();
        std::cout << "Notification from " << serialize(owner.stateId()) << ":" << owner.framesInState() << std::endl;
    }


    template<typename StateIDT, typename ViewT>
    UpdateGravity<StateIDT, ViewT>::UpdateGravity(TimelineProperty<Vector2<float>> &&gravity_) :
        m_gravity{std::move(gravity_)}
    {}

    template<typename StateIDT, typename ViewT>
    void UpdateGravity<StateIDT, ViewT>::operator()(const ViewT &view_) const
    {
        auto &physical = view_.template get<ComponentPhysical>();
        const SM::StatePossessor<StateIDT> &owner = view_.template cget<SM::StatePossessor<StateIDT>>();
        physical.m_gravity = m_gravity[owner.framesInState()];
    }
}


namespace SM::Transitions::Rules
{
    namespace In
    {
        template<typename StateIDT, typename ViewT>
        void Notify<StateIDT, ViewT>::operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const
        {
            std::cout << "Transition (IN) " << serialize(transition_.fromState) << " => " << serialize(transition_.intoState) << " (" << serialize(transition_.intoOrientation) << ")" << std::endl;
        }

        template<typename StateIDT, typename ViewT>
        SetGravity<StateIDT, ViewT>::SetGravity(const Vector2<float> &gravity_) :
            m_gravity{gravity_}
        {}

        template<typename StateIDT, typename ViewT>
        void SetGravity<StateIDT, ViewT>::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
        {
            view_.template get<ComponentPhysical>().m_gravity = m_gravity;
        }

        template<typename StateIDT, typename ViewT>
        SetAnimation<StateIDT, ViewT>::SetAnimation(ResID anim_) :
            m_anim{anim_}
        {}

        template<typename StateIDT, typename ViewT>
        void SetAnimation<StateIDT, ViewT>::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
        {
            auto &renderable = view_.template get<ComponentAnimationRenderable>();
            renderable.m_currentAnimation = &renderable.m_animations.at(m_anim);
            renderable.m_currentAnimation->reset();
        }
    }

    namespace Out
    {
        template<typename StateIDT, typename ViewT>
        void Notify<StateIDT, ViewT>::operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const
        {
            std::cout << "Transition (OUT) " << serialize(transition_.fromState) << " => " << serialize(transition_.intoState) << " (" << serialize(transition_.intoOrientation) << ")" << std::endl;
        }
    }
}

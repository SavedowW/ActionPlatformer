#include "Core/CoreComponents.h"
#include "Core/InputResolver.h"
#include "Core/InputState.h"
#include "Core/Vector2.hpp"
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

    template<typename StateIDT, typename ViewT>
    AddOrientedVelocity<StateIDT, ViewT>::AddOrientedVelocity(TimelineProperty<Vector2<float>> &&velocity_) :
        m_velocity{std::move(velocity_)}
    {}

    template<typename StateIDT, typename ViewT>
    void AddOrientedVelocity<StateIDT, ViewT>::operator()(const ViewT &view_) const
    {
        auto &physical = view_.template get<ComponentPhysical>();
        auto &transform = view_.template get<ComponentTransform>();
        auto velocityChange = m_velocity[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
        if (transform.m_orientation == ORIENTATION::LEFT)
            velocityChange.x *= -1;
        physical.m_velocity += velocityChange;
    }

    template<typename StateIDT, typename ViewT>
    HorizontalVelocityLimit<StateIDT, ViewT>::HorizontalVelocityLimit(TimelineProperty<std::pair<float, float>> &&limits_) :
        m_limits{std::move(limits_)}
    {}

    template<typename StateIDT, typename ViewT>
    void HorizontalVelocityLimit<StateIDT, ViewT>::operator()(const ViewT &view_) const
    {
        auto &physical = view_.template get<ComponentPhysical>();
        const auto &currentLimit = m_limits[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
        if (physical.m_velocity.x < currentLimit.first)
            physical.m_velocity.x = currentLimit.first;
        else if (physical.m_velocity.x > currentLimit.second)
            physical.m_velocity.x = currentLimit.second;
    }


    template<typename StateIDT, typename ViewT>
    SetDrag<StateIDT, ViewT>::SetDrag(TimelineProperty<Vector2<float>> &&drag_) :
        m_drag{std::move(drag_)}
    {}

    template<typename StateIDT, typename ViewT>
    void SetDrag<StateIDT, ViewT>::operator()(const ViewT &view_) const
    {
        auto &physical = view_.template get<ComponentPhysical>();
        const auto &current = m_drag[view_.template cget<SM::StatePossessor<StateIDT>>().framesInState()];
        physical.m_drag = current;
    }


    template<typename StateIDT, typename ViewT>
    void TestFallthrough<StateIDT, ViewT>::operator()(const ViewT &view_) const
    {
        if (view_.template get<InputResolver>().isInputActive(INPUT_BUTTON::DOWN))
            view_.template get<ComponentObstacleFallthrough>().setIgnoringObstacles();
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

        template<typename StateIDT, typename ViewT>
        void Realign<StateIDT, ViewT>::operator()(const ViewT &view_, const SM::TransitionData<StateIDT> &transition_) const
        {
            auto &transform = view_.template get<ComponentTransform>();
            transform.m_orientation = transition_.intoOrientation;
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

    template<typename StateIDT, typename ViewT>
    AddOrientedVelocity<StateIDT, ViewT>::AddOrientedVelocity(const Vector2<float> &velocity_) :
        m_velocity{velocity_}
    {}

    template<typename StateIDT, typename ViewT>
    void AddOrientedVelocity<StateIDT, ViewT>::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
    {
        auto &physical = view_.template get<ComponentPhysical>();
        auto &transform = view_.template get<ComponentTransform>();
        physical.m_velocity.y += m_velocity.y;
        if (transform.m_orientation == ORIENTATION::RIGHT)
            physical.m_velocity.x += m_velocity.x;
        else
            physical.m_velocity.x -= m_velocity.x;
    }


    template<typename StateIDT, typename ViewT>
    SetDrag<StateIDT, ViewT>::SetDrag(const Vector2<float> &drag_) :
        m_drag{drag_}
    {}

    template<typename StateIDT, typename ViewT>
    void SetDrag<StateIDT, ViewT>::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
    {
        view_.template get<ComponentPhysical>().m_drag = m_drag;
    }


    template<typename StateIDT, typename ViewT>
    ConvertToInertia<StateIDT, ViewT>::ConvertToInertia(bool convertVelocity_, bool includeEnforced_) :
        m_convertVelocity{convertVelocity_},
        m_includeEnforced{includeEnforced_}
    {}

    template<typename StateIDT, typename ViewT>
    void ConvertToInertia<StateIDT, ViewT>::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
    {
        view_.template get<ComponentPhysical>().convertToInertia(m_convertVelocity, m_includeEnforced);
    }


    template<typename StateIDT, typename ViewT>
    SetInertiaApplicationMultiplier<StateIDT, ViewT>::SetInertiaApplicationMultiplier(const Vector2<float> &mul_) :
        m_mul{mul_}
    {}

    template<typename StateIDT, typename ViewT>
    void SetInertiaApplicationMultiplier<StateIDT, ViewT>::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
    {
        view_.template get<ComponentPhysical>().m_inertiaMultiplier = m_mul;
    }


    template<typename StateIDT, typename ViewT>
    SetMagnetLimit<StateIDT, ViewT>::SetMagnetLimit(unsigned int magnetLimit_) :
        m_magnetLimit{magnetLimit_}
    {}

    template<typename StateIDT, typename ViewT>
    void SetMagnetLimit<StateIDT, ViewT>::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
    {
        view_.template get<ComponentPhysical>().m_magnetLimit = m_magnetLimit;
    }


    template<typename StateIDT, typename ViewT>
    void TestFallthrough<StateIDT, ViewT>::operator()(const ViewT &view_, const SM::TransitionData<StateIDT>&) const
    {
        if (view_.template get<InputResolver>().isInputActive(INPUT_BUTTON::DOWN))
            view_.template get<ComponentObstacleFallthrough>().setIgnoringObstacles();
    }
}

#pragma once
#include "Core/Vector2.hpp"
#include "Core/TimelineProperty.hpp"
#include "StateMachine.h"

namespace SM::Updaters
{
    template<typename StateIDT, typename ViewT>
    class Notify
    {
    public:
        void operator()(const ViewT &view_) const;
    };

    template<typename StateIDT, typename ViewT>
    class UpdateGravity
    {
    public:
        UpdateGravity(TimelineProperty<Vector2<float>> &&gravity_);

        void operator()(const ViewT &view_) const;
    
    private:
        TimelineProperty<Vector2<float>> m_gravity;
    };

    template<typename StateIDT, typename ViewT>
    class AddOrientedVelocity
    {
    public:
        AddOrientedVelocity(TimelineProperty<Vector2<float>> &&velocity_);

        void operator()(const ViewT &view_) const;
    
    private:
        TimelineProperty<Vector2<float>> m_velocity;
    };

    template<typename StateIDT, typename ViewT>
    class HorizontalVelocityLimit
    {
    public:
        HorizontalVelocityLimit(TimelineProperty<std::pair<float, float>> &&limits_);

        void operator()(const ViewT &view_) const;
    
    private:
        TimelineProperty<std::pair<float, float>> m_limits;
    };

    template<typename StateIDT, typename ViewT>
    class SetDrag
    {
    public:
        SetDrag(TimelineProperty<Vector2<float>> &&drag_);

        void operator()(const ViewT &view_) const;
    
    private:
        TimelineProperty<Vector2<float>> m_drag;
    };

    template<typename StateIDT, typename ViewT>
    class TestFallthrough
    {
    public:
        void operator()(const ViewT&) const;
    };
}


namespace SM::Transitions::Rules
{
    namespace In
    {
        template<typename StateIDT, typename ViewT>
        class Notify
        {
        public:
            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        };

        template<typename StateIDT, typename ViewT>
        class SetGravity
        {
        public:
            SetGravity(const Vector2<float> &gravity_);
            void operator()(const ViewT &view_, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const Vector2<float> m_gravity;
        };

        template<typename StateIDT, typename ViewT>
        class SetAnimation
        {
        public:
            SetAnimation(ResID anim_);
            void operator()(const ViewT &view_, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const ResID m_anim;
        };

        template<typename StateIDT, typename ViewT>
        class Realign
        {
        public:
            void operator()(const ViewT &view_, const SM::TransitionData<StateIDT> &transition_) const;
        };
    }

    namespace Out
    {
        template<typename StateIDT, typename ViewT>
        class Notify
        {
        public:
            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        };
    }

    template<typename StateIDT, typename ViewT>
    class AddOrientedVelocity
    {
    public:
        AddOrientedVelocity(const Vector2<float> &velocity_);

        void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
    
    private:
        const Vector2<float> m_velocity;
    };

    template<typename StateIDT, typename ViewT>
    class SetDrag
    {
    public:
        SetDrag(const Vector2<float> &drag_);

        void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
    
    private:
        const Vector2<float> m_drag;
    };

    template<typename StateIDT, typename ViewT>
    class ConvertToInertia
    {
    public:
        ConvertToInertia(bool convertVelocity_, bool includeEnforced_);

        void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
    
    private:
        const bool m_convertVelocity;
        const bool m_includeEnforced;
    };

    template<typename StateIDT, typename ViewT>
    class SetInertiaApplicationMultiplier
    {
    public:
        SetInertiaApplicationMultiplier(const Vector2<float> &mul_);

        void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
    
    private:
        const Vector2<float> m_mul;
    };

    template<typename StateIDT, typename ViewT>
    class SetMagnetLimit
    {
    public:
        SetMagnetLimit(unsigned int magnetLimit_);

        void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
    
    private:
        const unsigned int m_magnetLimit;
    };

    template<typename StateIDT, typename ViewT>
    class TestFallthrough
    {
    public:
        void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
    };
}

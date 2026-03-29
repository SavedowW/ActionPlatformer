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
}

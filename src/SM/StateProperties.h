#pragma once
#include "Core/Vector2.hpp"
#include "Core/TimelineProperty.hpp"
#include "StateMachine.h"

template<typename StateIDT, typename ViewT>
struct StateProperties
{
    struct Update
    {
        class Notify
        {
        public:
            void operator()(const ViewT &view_) const;
        };

        class UpdateGravity
        {
        public:
            UpdateGravity(TimelineProperty<Vector2<float>> &&gravity_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<Vector2<float>> m_gravity;
        };


        class MultiplyVelocity
        {
        public:
            MultiplyVelocity(TimelineProperty<Vector2<float>> &&multiplier_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<Vector2<float>> m_multiplier;
        };

        class AddOrientedVelocity
        {
        public:
            AddOrientedVelocity(TimelineProperty<Vector2<float>> &&velocity_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<Vector2<float>> m_velocity;
        };

        class AddAbsoluteVelocity
        {
        public:
            AddAbsoluteVelocity(TimelineProperty<Vector2<float>> &&velocity_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<Vector2<float>> m_velocity;
        };

        class MultiplyInertia
        {
        public:
            MultiplyInertia(TimelineProperty<Vector2<float>> &&multiplier_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<Vector2<float>> m_multiplier;
        };


        class HorizontalVelocityLimit
        {
        public:
            HorizontalVelocityLimit(TimelineProperty<std::pair<float, float>> &&limits_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<std::pair<float, float>> m_limits;
        };

        class VerticalVelocityLimit
        {
        public:
            VerticalVelocityLimit(TimelineProperty<std::pair<float, float>> &&limits_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<std::pair<float, float>> m_limits;
        };

        class HorizontalInertiaLimit
        {
        public:
            HorizontalInertiaLimit(TimelineProperty<std::pair<float, float>> &&limits_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<std::pair<float, float>> m_limits;
        };

        class AirDrift
        {
        public:
            void operator()(const ViewT &view_) const;
        };


        class SetDrag
        {
        public:
            SetDrag(TimelineProperty<Vector2<float>> &&drag_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<Vector2<float>> m_drag;
        };


        class LookaheadSpeedSensitivity
        {
        public:
            LookaheadSpeedSensitivity(TimelineProperty<Vector2<float>> &&sensitivity_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<Vector2<float>> m_sensitivity;
        };


        class TestFallthrough
        {
        public:
            void operator()(const ViewT&) const;
        };

        class MagnetLimit
        {
        public:
            MagnetLimit(TimelineProperty<unsigned int> &&magnetLimit_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<unsigned int> m_magnetLimit;
        };


        class Realign
        {
        public:
            Realign(TimelineProperty<bool> &&shouldRealign_);

            void operator()(const ViewT &view_) const;
        
        private:
            TimelineProperty<bool> m_shouldRealign;
        };
    };


    struct Pipe
    {
        class Notify
        {
        public:
            Notify(const char *action_);
            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;

        private:
            const char *m_action;
        };

        class SetGravity
        {
        public:
            SetGravity(const Vector2<float> &gravity_);
            void operator()(const ViewT &view_, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const Vector2<float> m_gravity;
        };

        class SetAnimation
        {
        public:
            SetAnimation(ResID anim_);
            void operator()(const ViewT &view_, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const ResID m_anim;
        };

        class Realign
        {
        public:
            void operator()(const ViewT &view_, const SM::TransitionData<StateIDT> &transition_) const;
        };


        class MultiplyVelocity
        {
        public:
            MultiplyVelocity(const Vector2<float> &multiplier_);

            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const Vector2<float> m_multiplier;
        };

        class AddOrientedVelocity
        {
        public:
            AddOrientedVelocity(const Vector2<float> &velocity_);

            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const Vector2<float> m_velocity;
        };


        class SetDrag
        {
        public:
            SetDrag(const Vector2<float> &drag_);

            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const Vector2<float> m_drag;
        };

        class ConvertToInertia
        {
        public:
            ConvertToInertia(bool convertVelocity_, bool includeEnforced_);

            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const bool m_convertVelocity;
            const bool m_includeEnforced;
        };

        class SetInertiaApplicationMultiplier
        {
        public:
            SetInertiaApplicationMultiplier(const Vector2<float> &mul_);

            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const Vector2<float> m_mul;
        };

        class SetMagnetLimit
        {
        public:
            SetMagnetLimit(unsigned int magnetLimit_);

            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const unsigned int m_magnetLimit;
        };

        class TestFallthrough
        {
        public:
            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        };

        class HaltSideDownwardMomentum
        {
        public:
            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        };

        class SetDemandWall
        {
        public:
            constexpr SetDemandWall(bool demandWall_);
            
            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;

        private:
            bool m_demandWall;
        };

        class LeaveWallPrejump
        {
        public:
            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        };

        class SetLookaheadSpeedSensitivity
        {
        public:
            constexpr SetLookaheadSpeedSensitivity(const Vector2<float> &sensitivity_);

            void operator()(const ViewT&, const SM::TransitionData<StateIDT> &transition_) const;
        
        private:
            const Vector2<float> m_sensitivity;
        };
    };
};

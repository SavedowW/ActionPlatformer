#pragma once
#include "Core/CoreComponents.h"
#include "Core/InputResolver.h"
#include "Core/StateCommon.h"
#include "StateMachine.h"
#include <vector>

enum class OrientationOptions : uint8_t
{
    NONE     = 0b0000,
    LEFT     = 0b0001,
    RIGHT    = 0b0010,
    SAME     = 0b0100,
    OPPOSITE = 0b1000,
};

template<typename StateIDT, typename ViewT>
struct TransitionChecks
{
    struct Base
    {
        class AbstractCondition
        {
        public:
            AbstractCondition() = default;
            AbstractCondition(const AbstractCondition &) = default;
            AbstractCondition &operator=(const AbstractCondition &) = default;
            AbstractCondition(AbstractCondition &&) = default;
            AbstractCondition &operator=(AbstractCondition &&) = default;
            virtual ~AbstractCondition() = default;

            virtual operator StateIDT() const = 0;
            virtual ORIENTATION operator()(const ViewT &view_) = 0;
        };

        class Sequential
        {
        public:
            Sequential &addCondition(std::unique_ptr<AbstractCondition> &&condition_);
            SM::TransitionData<StateIDT> operator()(const ViewT &view_) const;

        private:
            std::vector<std::unique_ptr<AbstractCondition>> m_conditions;
        };


        class AbstractStateCondition : public AbstractCondition
        {
        public:
            AbstractStateCondition(StateIDT state_);
            operator StateIDT() const override;

        private:
            const StateIDT m_state;
        };


        template<typename T>
        class SinceFrameImpl : public AbstractCondition
        {
        public:
            SinceFrameImpl(uint32_t sinceFrame_, T &&condition_);
            operator StateIDT() const override;

            ORIENTATION operator()(const ViewT &view_) override;

        private:
            const uint32_t m_sinceFrame;
            T m_condition;
        };
    };

    template<typename T>
    static auto sinceFrame(uint32_t sinceFrame_, T &&condition_);


    class OnPhysEvent : public Base::AbstractStateCondition
    {
    public:
        OnPhysEvent(const StateIDT &state_, const PhysicalEvents::Events &event_, bool isSet_ = true);

        ORIENTATION operator()(const ViewT &view_) override;

    private:
        const PhysicalEvents::Events m_event;
        const bool m_isSet;
    };

    class OnGrounded : public Base::AbstractStateCondition
    {
    public:
        OnGrounded(const StateIDT &state_, bool isGrounded_ = true);

        ORIENTATION operator()(const ViewT &view_) override;

    private:
        const bool m_isGrounded;
    };

    class OnTimer : public Base::AbstractStateCondition
    {
    public:
        OnTimer(const StateIDT &state_, uint32_t framesLimit_);

        ORIENTATION operator()(const ViewT &view_) override;

    private:
        const uint32_t m_framesLimit;
    };

    class InputTest : public Base::AbstractStateCondition
    {
    public:
        InputTest(const StateIDT &state_, InputMotions input_, Flag<OrientationOptions> options_);

        ORIENTATION operator()(const ViewT &view_) override;

    private:
        const InputMotions m_input;
        const Flag<OrientationOptions> m_orientations;
    };
};

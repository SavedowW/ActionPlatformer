#pragma once
#include "Core/CoreComponents.h"
#include "Core/InputResolver.h"
#include "Core/Flag.h"
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
            virtual Flag<Orientation> operator()(const ViewT &view_) = 0;
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

            Flag<Orientation> operator()(const ViewT &view_) override;

        private:
            const uint32_t m_sinceFrame;
            T m_condition;
        };
    };

    template<typename T>
    static auto sinceFrame(uint32_t sinceFrame_, T &&condition_);

    class OnGrounded : public Base::AbstractStateCondition
    {
    public:
        OnGrounded(const StateIDT &state_, bool isGrounded_ = true);

        Flag<Orientation> operator()(const ViewT &view_) override;

    private:
        const bool m_isGrounded;
    };

    // Only true if really is grounded AND speed is equal or greater to the condition
    class OnGroundedBySpeed : public Base::AbstractStateCondition
    {
    public:
        OnGroundedBySpeed(const StateIDT &state_, bool isGrounded_, float minVerSpeed_);

        Flag<Orientation> operator()(const ViewT &view_) override;

    private:
        const bool m_isGrounded;
        const float m_minVerSpeed;
    };

    class OnTimer : public Base::AbstractStateCondition
    {
    public:
        OnTimer(const StateIDT &state_, uint32_t framesLimit_);

        Flag<Orientation> operator()(const ViewT &view_) override;

    private:
        const uint32_t m_framesLimit;
    };

    class InputTest : public Base::AbstractStateCondition
    {
    public:
        InputTest(const StateIDT &state_, InputMotions input_, Flag<OrientationOptions> options_, uint32_t bufferExtention_ = 0);

        Flag<Orientation> operator()(const ViewT &view_) override;

    private:
        const InputMotions m_input;
        const Flag<OrientationOptions> m_orientations;
        const uint32_t m_bufferExtention;
    };


    class WallClingEnterTest : public Base::AbstractStateCondition
    {
    public:
        WallClingEnterTest(const StateIDT &state_);

        Flag<Orientation> operator()(const ViewT &view_) override;
    };

    class WallClingLeaveTest : public Base::AbstractStateCondition
    {
    public:
        WallClingLeaveTest(const StateIDT &state_);

        Flag<Orientation> operator()(const ViewT &view_) override;
    };
};

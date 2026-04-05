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

namespace SM
{
    template<typename StateIDT, typename ViewT>
    class AbstractCondition
    {
    public:
        AbstractCondition(const StateIDT &state_);
        operator StateIDT() const;

        virtual ORIENTATION operator()(const ViewT &view_) = 0;

        virtual ~AbstractCondition() = default;

    private:
        const StateIDT m_state;
    };

    template<typename StateIDT, typename ViewT>
    class Sequential
    {
    public:
        Sequential &addCondition(std::unique_ptr<AbstractCondition<StateIDT, ViewT>> &&condition_);
        TransitionData<StateIDT> operator()(const ViewT &view_) const;

    private:
        std::vector<std::unique_ptr<AbstractCondition<StateIDT, ViewT>>> m_conditions;
    };


    namespace Transition::Checks
    {
        template<typename StateIDT, typename ViewT>
        class OnPhysEvent : public AbstractCondition<StateIDT, ViewT>
        {
        public:
            OnPhysEvent(const StateIDT &state_, const PhysicalEvents::Events &event_, bool isSet_ = true);

            ORIENTATION operator()(const ViewT &view_) override;

        private:
            const PhysicalEvents::Events m_event;
            const bool m_isSet;
        };

        template<typename StateIDT, typename ViewT>
        class OnTimer : public AbstractCondition<StateIDT, ViewT>
        {
        public:
            OnTimer(const StateIDT &state_, uint32_t framesLimit_);

            ORIENTATION operator()(const ViewT &view_) override;

        private:
            const uint32_t m_framesLimit;
        };

        template<typename StateIDT, typename ViewT>
        class InputTest : public AbstractCondition<StateIDT, ViewT>
        {
        public:
            InputTest(const StateIDT &state_, InputMotions input_, Flag<OrientationOptions> options_);

            ORIENTATION operator()(const ViewT &view_) override;

        private:
            const InputMotions m_input;
            const Flag<OrientationOptions> m_orientations;
        };
    }
}

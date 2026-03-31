#pragma once
#include "Core/CoreComponents.h"
#include "StateMachine.h"
#include <vector>

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
            OnPhysEvent(const StateIDT &state_, const PhysicalEvents::Events &event_);

            ORIENTATION operator()(const ViewT &view_) override;

        private:
            const PhysicalEvents::Events m_event;
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
    }
}

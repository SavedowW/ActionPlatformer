#pragma once
#include "SM/PhysicalState.hpp"
#include "SM/Builder.hpp"
#include "SM/CompoundState.hpp"
#include "SM/StateMachine.hpp"
#include "Core/Vector2.hpp"
#include "Core/CoreComponents.h"
#include "Core/ECS/ComponentsView.hpp"
#include <iostream>

enum class TestStates : SM::StateID {
    IDLE,
    RUN,
    ATTACK
};

SERIALIZE_ENUM(TestStates, {
    ENUM_AUTO(TestStates, IDLE),
    ENUM_AUTO(TestStates, RUN),
    ENUM_AUTO(TestStates, ATTACK)
})

using TestView = ComponentsView<SM::StatePossessor<TestStates>, ComponentTransform, ComponentPhysical, ComponentAnimationRenderable>;
using TestMake = SM::Make<TestStates, TestView>;


void testNewStateMachine()
{
    entt::registry reg;
    auto ent = reg.create();
    reg.emplace<SM::StatePossessor<TestStates>>(ent, TestStates::IDLE);
    reg.emplace<ComponentTransform>(ent, Vector2{780, 470}, ORIENTATION::RIGHT);
    reg.emplace<ComponentPhysical>(ent);
    reg.emplace<ComponentAnimationRenderable>(ent);

    auto view = TestView::makeView(reg);
    for (const auto &data : view.each())
    {
        TestView entityView{data};
        entityView.get<ComponentTransform>();
    }

    SM::StateMachine<TestStates, TestView> sm;
    
    sm.addState(
        TestMake::state(
            TestStates::IDLE,

            SM::CallBatch(
                SM::Updaters::Notify<TestStates, TestView>()),

            [](const TestView &view_) {
                std::cout << "Idle asked" << std::endl;
                return SM::TransitionData<TestStates>{TestStates::IDLE, TestStates::RUN, view_.get<ComponentTransform>().m_orientation};
            },

            TestMake::RulePipe{}
                .setDefaultPipe(SM::Transitions::Rules::Out::Notify<TestStates, TestView>{})
                .setPipe(TestStates::RUN, SM::Transitions::Rules::Out::Notify<TestStates, TestView>{})
                .done(),

            TestMake::RulePipe{}
                .setDefaultPipe(SM::Transitions::Rules::In::Notify<TestStates, TestView>{})
                .setPipe(TestStates::RUN, SM::Transitions::Rules::In::Notify<TestStates, TestView>{})
                .done()
    ));

    sm.addState(
        TestMake::state(
            TestStates::RUN,

            SM::CallBatch(
                SM::Updaters::Notify<TestStates, TestView>()),

            [](const TestView&) {
                std::cout << "Run asked" << std::endl;
                return SM::TransitionData<TestStates>{TestStates::RUN, TestStates::RUN, ORIENTATION::UNSPECIFIED};
            },

            TestMake::RulePipe{}
                .setDefaultPipe(SM::Transitions::Rules::Out::Notify<TestStates, TestView>{})
                .setPipe(TestStates::IDLE, SM::Transitions::Rules::Out::Notify<TestStates, TestView>{})
                .done(),

            TestMake::RulePipe{}
                .setDefaultPipe(SM::Transitions::Rules::In::Notify<TestStates, TestView>{})
                .setPipe(TestStates::IDLE, SM::Transitions::Rules::In::Notify<TestStates, TestView>{})
                .done()
    ));

    sm.update(reg);
    sm.update(reg);
    sm.update(reg);

    AbstractCallable<int, double>::Compound callIt([](int i, double d){
        std::cout << i + d << std::endl;
    }, [](int i, double d){
        std::cout << i - d << std::endl;
    });

    callIt(1, 2.3);

}

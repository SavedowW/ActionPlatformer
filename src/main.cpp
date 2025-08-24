#include "Core/CallChain.h"
#include "Core/Utils.hpp"
#include "StateMachineEx.hpp"
#include "Core/CallChain.hpp"
#include "Core/CoreComponents.h"
#include "Core/StaticMapping.hpp"
#include "Stage1.h"
#include "Core/Application.h"
#include "Core/FilesystemUtils.h"
#include "entt/entity/fwd.hpp"
#include <bitset>
#include <algorithm>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

#define EXPERIMENTS

#ifdef EXPERIMENTS
#endif

int main(int, char**)
{
#ifdef EXPERIMENTS

    entt::registry  reg;
    entt::entity ent = reg.create();
    reg.emplace<ComponentTransform>(ent);
    reg.emplace<ComponentPhysical>(ent);
    reg.emplace<ComponentAnimationRenderable>(ent);

#if 0
    callable::Chain chain([](const char *i_){
        return std::atoi(i_);
    }, Multiplier(2.5f),
    [](auto i_){
        return i_ * 0.5f;
    }, [](auto i_){
        std::cout <<  i_ + 1.7 << std::endl;
    });

    chain("1");
    chain("2");

    callable::EachResolved eachChain(UpdateVelocity{1.0f, -5.118f}, UpdatePos{5, 13}, ChangeAnim{123123});
    eachChain(reg.get<ComponentTransform, ComponentPhysical, ComponentAnimationRenderable>(ent));

    return 0;
#endif

    EStateMachine sm("root",
        CompoundState(
            UpdateBody(UpdateVelocity{.x=1.0f, .y=-5.118f}, ChangeAnim{123123}),
            formInPipeSet(
                CompoundInPipe(UpdateVelocity{.x=0.0f, .y=-5.0f}),
                CompoundInPipe())
        )
        //CompoundState(UpdateVelocity{}),
        //CompoundState(UpdateVelocity{}, UpdatePos{}, ChangeAnim{})
    );

    sm.dump(std::cout, 0);

    decltype(sm)::Iterator it(sm);

    sm.update(it, {.reg=&reg, .idx=ent});

    std::cout << "Results: " << std::endl;

    std::cout << reg.get<ComponentTransform>(ent).m_pos << ", " << serialize(reg.get<ComponentTransform>(ent).m_orientation) << std::endl;
    std::cout << reg.get<ComponentPhysical>(ent).m_velocity << std::endl;
    std::cout << reinterpret_cast<ResID>(reg.get<ComponentAnimationRenderable>(ent).m_currentAnimation) << std::endl;

    std::cout << "Success!" << std::endl;
    return 0;

#else

    try
    {
        std::cout << Filesystem::getRootDirectory() << std::endl;
        auto &app = Application::instance();
        
        app.setLevel(1, std::make_unique<Stage1>(Vector2{2048, 2048}, 1));
        app.run();
    }
    catch (std::exception &ex_)
    {
        std::cout << "Something went horribly wrong!" << std::endl;
        std::cout << ex_.what() << std::endl;

        return 1;
    }

    return 0;

#endif
}

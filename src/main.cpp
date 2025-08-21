#include "StateMachineEx.hpp"
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


int main(int, char**)
{
    std::cout << std::bitset<8>(int8_t(0b01)) << std::endl;
    std::cout << std::bitset<8>(int8_t(-2)) << std::endl;
#ifdef EXPERIMENTS

    entt::registry  reg;
    entt::entity ent = reg.create();
    reg.emplace<ComponentTransform>(ent);
    reg.emplace<ComponentPhysical>(ent);
    reg.emplace<ComponentAnimationRenderable>(ent);

    EStateMachine sm( "root",
        CompoundState(UpdateVelocity{}, ChangeAnim{}),
        CompoundState(UpdateVelocity{}),
        CompoundState(UpdateVelocity{}, UpdatePos{}, ChangeAnim{})
    );

    sm.dump(std::cout, 0);

    decltype(sm)::Iterator it(sm);

    sm.update(it, {.reg=&reg, .idx=ent});

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

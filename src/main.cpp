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

template<typename... Fs>
auto callChain(const std::tuple<Fs...> &tuple_)
{
    return std::get<2>(tuple_)(std::get<1>(tuple_)(std::get<0>(tuple_)(1)));
}

template<size_t idx = 0, typename... TCallable>
constexpr auto callRecursive(std::tuple<TCallable...> &tuple_, const auto &default_)
{
    if constexpr (idx == sizeof...(TCallable) - 1)
        return std::get<sizeof...(TCallable) - idx - 1>(tuple_)(default_);
    else
        return std::get<sizeof...(TCallable) - idx - 1>(tuple_)(callRecursive<idx + 1>(tuple_, default_));
}

int main(int, char**)
{

    std::tuple tpl([](const char *i_){
        return std::atoi(i_);
    }, [](int i_){
        return i_ * 0.5f;
    }, [](float i_){
        return i_ + 0.5;
    });

    std::cout << callRecursive(tpl, "1") << std::endl;

    return 0;

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

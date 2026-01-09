#include "tests/NewStateMachine.hpp"
#include "tests/ComponentExtraction.hpp"
#include "Core/CoreComponents.h"
#include "BattleLevel.h"
#include "Core/Application.hpp"
#include "Core/FilesystemUtils.h"
#include <iostream>
#include <memory>

//#define EXPERIMENTS

int main(int, char**)
{
#ifdef EXPERIMENTS

    try
    {
        //testCompExtraction();
        testNewStateMachine();
    }
    catch (std::exception &ex_)
    {
        std::cout << "Exception while messing around!" << std::endl;
        std::cout << ex_.what() << std::endl;

        return 1;
    }

#else

    try
    {
        std::cout << Filesystem::getRootDirectory() << std::endl;
        auto &app = Application::instance();
        
        app.makeLevel<BattleLevel>(1, Vector2{2048, 2048});
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

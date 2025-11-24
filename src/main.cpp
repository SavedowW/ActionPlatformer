#include "tests/ComponentExtraction.hpp"
#include "Core/CoreComponents.h"
#include "Stage1.h"
#include "Core/Application.h"
#include "Core/FilesystemUtils.h"
#include <iostream>
#include <memory>

//#define EXPERIMENTS

int main(int, char**)
{
#ifdef EXPERIMENTS

    try
    {
        testCompExtraction();
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

#include "Stage1.h"
#include "Application.h"
#include "FilesystemUtils.h"

#include <iostream>

//#define EXPERIMENTS

int main(int, char**)
{
#ifdef EXPERIMENTS

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

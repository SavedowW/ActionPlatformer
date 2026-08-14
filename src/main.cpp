#include "Core/Logger.hpp" // IWYU pragma: keep
#include "BattleLevel.h"
#include "Core/Application.hpp"
#include "Core/FilesystemUtils.h"

//#define EXPERIMENTS

#ifdef EXPERIMENTS
#include "tests/NewStateMachine.hpp"  // IWYU pragma: keep
#endif

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
        LOG_INFO(Filesystem::getRootDirectory());
        auto &app = Application::instance();
        
        app.makeLevel<BattleLevel>("Tilemaps/Level1.json");
        app.makeLevel<BattleLevel>("Tilemaps/LevelTest.json");
        app.run();
    }
    catch (std::exception &ex_)
    {
        LOG_ERROR("Something went horribly wrong!\n{}", ex_.what());

        return 1;
    }

    return 0;

#endif
}

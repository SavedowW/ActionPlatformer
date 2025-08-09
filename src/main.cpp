#include "Stage1.h"
#include "Application.h"
#include "FilesystemUtils.h"
#include "Configuration.h"

#include <iostream>
#include <string>
#include <stack>

int main(int, char**)
{
    try
    {
        std::cout << Filesystem::getRootDirectory() << std::endl;
        Application app;
        
        app.setLevel(1, new Stage1(app, {2048, 2048}, 1));
        app.run();
    }
    catch (std::exception &ex_)
    {
        std::cout << "Something went horribly wrong!" << std::endl;
        std::cout << ex_.what() << std::endl;

        return 1;
    }

    return 0;
}

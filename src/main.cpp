#include "Stage1.h"
#include "Application.h"
#include "FilesystemUtils.h"
#include "Configuration.h"

#include <iostream>
#include <string>
#include <stack>

int main(int argc, char* args[])
{
    std::cout << Filesystem::getRootDirectory() << std::endl;
    Application app;
    
    app.setLevel(1, new Stage1(&app, {2048, 2048}, 1));
    app.run();

    return 0;
}

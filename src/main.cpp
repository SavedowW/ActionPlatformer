#include "Stage1.h"
#include "Application.h"
#include "FilesystemUtils.h"

#include <iostream>
#include <string>

int main(int argc, char* args[])
{    
    Application app;
    std::cout << Filesystem::getRootDirectory() << std::endl;
    //RecipeParser pr(app.getAnimationManager(), app.getBasePath() + "/Resources/data.json");

    app.setLevel(1, new Stage1(&app, {2048, 2048}, 1));
    app.run();

    return 0;
}
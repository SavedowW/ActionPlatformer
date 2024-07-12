#include <iostream>
#include <string>

#include "Stage1.h"
#include "Application.h"
#include "TimelineProperty.h"

void f(float v)
{
    std::cout << v << std::endl;
}

int main(int argc, char* args[])
{    
    Application app;
    std::cout << app.getBasePath() << std::endl;
    //RecipeParser pr(app.getAnimationManager(), app.getBasePath() + "/Resources/data.json");

    app.setLevel(1, new Stage1(&app, {1024, 1024}, 1));
    app.run();

    return 0;
}
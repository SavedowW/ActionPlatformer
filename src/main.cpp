#include "Stage1.h"
#include "Application.h"
#include "FilesystemUtils.h"
#include "EngineAnimation.h"

#include <iostream>
#include <string>

int main(int argc, char* args[])
{    
    Application app;
    std::cout << Filesystem::getRootDirectory() << std::endl;
    //RecipeParser pr(app.getAnimationManager(), app.getBasePath() + "/Resources/data.json");

    
    std::string path = "C:/stuff/ActionPlatformer/Resources/Sprites/Environment/grass_single_top_flickR.panm";
    EngineAnimation anim;
	anim.loadAnimation(path);
    
    
    std::cout << "{" << std::endl;
    std::cout << "    \"origin_x\":" << anim.m_origin.x << "," << std::endl;
    std::cout << "    \"origin_y\":" << anim.m_origin.y << "," << std::endl;
    std::cout << "    \"duration\":" << anim.m_duration << "," << std::endl;
    anim.m_framesData.dump();
    std::cout << "}" << std::endl;
    
    ///*
    app.setLevel(1, new Stage1(&app, {2048, 2048}, 1));
    app.run();
    //*/

    return 0;
}
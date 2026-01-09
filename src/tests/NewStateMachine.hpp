#pragma once
#include "StateMachineEx.h"
#include "Core/CoreComponents.h"
#include "Core/ECS/ComponentRegistry.h"
#include <iostream>

void testNewStateMachine()
{
    ComponentRegistry compReg;
    compReg.initExtractorFactory<ComponentTransform>();
    compReg.initExtractorFactory<ComponentPhysical>();
    compReg.initExtractorFactory<ComponentAnimationRenderable>();

    std::cout << compReg << std::endl;
    
    SM::StateMachine sm;
}

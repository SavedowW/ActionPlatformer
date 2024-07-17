#include "MovableCharacter.h"

MovableCharacter::MovableCharacter(Application &application_, const CollisionArea &cldArea_) :
    ComponentEntity(ComponentTransform(), ComponentPhysical(cldArea_), ComponentObstacleFallthrough())
{
    resolveDeps();
}


void MovableCharacter::resolveDeps()
{
    getComponent<ComponentPhysical>().ResolveDeps(&getComponent<ComponentTransform>(), &getComponent<ComponentObstacleFallthrough>());
    getComponent<ComponentObstacleFallthrough>().ResolveDeps(&getComponent<ComponentPhysical>());
}

void MovableCharacter::update()
{
    onUpdate();
}


#include "MovableCharacter.h"

MovableCharacter::MovableCharacter(Application &application_, const CollisionArea &cldArea_) :
    ComponentEntity(ComponentTransform(), ComponentPhysical(cldArea_), ComponentObstacleFallthrough(), ComponentAnimationRenderable(*application_.getRenderer()))
{
    resolveDeps();
}


void MovableCharacter::resolveDeps()
{
    getComponent<ComponentPhysical>().ResolveDeps(&getComponent<ComponentTransform>(), &getComponent<ComponentObstacleFallthrough>());
    getComponent<ComponentObstacleFallthrough>().ResolveDeps(&getComponent<ComponentPhysical>());
    getComponent<ComponentAnimationRenderable>().ResolveDeps(&getComponent<ComponentTransform>(), &getComponent<ComponentPhysical>());
}

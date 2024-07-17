#include "MovableCharacter.h"

MovableCharacter::MovableCharacter(Application &application_, const CollisionArea &cldArea_) :
    ComponentEntity(ComponentTransform(), ComponentPhysical(cldArea_), ComponentObstacleFallthrough())
{
    resolveDeps();
}

void MovableCharacter::setOnLevel(Application &application_, Vector2<float> pos_)
{
    loadAnimations(application_);
    getComponent<ComponentTransform>().m_pos = pos_;
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


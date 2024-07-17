#ifndef MOVABLE_CHARACTER_H_
#define MOVABLE_CHARACTER_H_
#include "CoreComponents.h"
#include "CollisionArea.h"
#include "Application.h"

class MovableCharacter : public ComponentEntity<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, ComponentAnimationRenderable>
{
public:
    MovableCharacter(Application &application_, const CollisionArea &cldArea_);

    virtual void setOnLevel(Application &application_, Vector2<float> pos_) = 0;
    virtual void update() = 0;

    virtual void onTouchedGround() = 0;
    virtual void onLostGround() = 0;
    virtual void resolveDeps() override;

protected:
};

#endif
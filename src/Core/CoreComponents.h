#ifndef CORE_COMPONENTS_H_
#define CORE_COMPONENTS_H_
#include "ComponentEntity.hpp"
#include "Vector2.h"
#include "FrameTimer.h"
#include <set>
#include "CollisionArea.h"

struct ComponentObstacleFallthrough;

struct ComponentTransform : public Component<>
{
    ComponentTransform() = default;

    Vector2<float> m_pos;

    ORIENTATION m_ownOrientation = ORIENTATION::RIGHT;

    Vector2<float> getOwnHorDir() const;
};

struct ComponentPhysical : public Component<ComponentTransform, ComponentObstacleFallthrough>
{
    ComponentPhysical(const CollisionArea &cldArea_);

    virtual void onUpdate() override;

    Vector2<float> m_velocity;
    Vector2<float> m_inertia;
    Vector2<float> m_drag; 
    Vector2<float> m_gravity;
    Vector2<float> m_inertiaMultiplier;
    Collider m_pushbox;
    const CollisionArea &m_collisionArea;
    float m_magnetLimit = 0.0f;
    float m_onSlopeWithAngle = 0.0f;

    void velocityToInertia();
    Collider getPushbox();
    Vector2<float> getPosOffest() const;
    bool attemptResetGround();
};

struct ComponentObstacleFallthrough : public Component<ComponentPhysical>
{
    ComponentObstacleFallthrough() = default;

    virtual void onUpdate() override;

    void setIgnoringObstacles();
    bool isIgnoringAllObstacles() const;
    void cleanIgnoredObstacles();
    bool touchedObstacleTop(int obstacleId_);
    bool touchedObstacleBottom(int obstacleId_);
    bool touchedObstacleSlope(int obstacleId_);
    bool touchedObstacleSide(int obstacleId_);
    bool checkIgnoringObstacle(int obstacleId_) const;

    FrameTimer<false> m_isIgnoringObstacles;
    std::set<int> m_ignoredObstacles;
};

#endif
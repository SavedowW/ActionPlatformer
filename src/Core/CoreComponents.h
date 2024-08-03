#ifndef CORE_COMPONENTS_H_
#define CORE_COMPONENTS_H_
#include "InputResolver.h"
#include "Vector2.h"
#include "FrameTimer.h"
#include "Renderer.h"
#include "Collider.h"
#include "AnimationManager.h"
#include "Trigger.h"
#include <set>
#include <map>
#include <memory>
#include <utility>

struct ComponentTransform
{
    ComponentTransform() = default;

    ComponentTransform(const Vector2<float> &pos_, ORIENTATION orient_);

    ComponentTransform (const ComponentTransform &rhs_) = delete;
    ComponentTransform (ComponentTransform &&rhs_) = default;
    ComponentTransform &operator=(const ComponentTransform &rhs_) = delete;
    ComponentTransform &operator=(ComponentTransform &&rhs_) = default;
    
    Vector2<float> m_pos;
    ORIENTATION m_orientation = ORIENTATION::RIGHT;
};

struct ComponentPhysical
{
    ComponentPhysical() = default;

    ComponentPhysical (const ComponentPhysical &rhs_) = delete;
    ComponentPhysical (ComponentPhysical &&rhs_) = default;
    ComponentPhysical &operator=(const ComponentPhysical &rhs_) = delete;
    ComponentPhysical &operator=(ComponentPhysical &&rhs_) = default;

    Vector2<float> m_velocity;
    Vector2<float> m_inertia;
    Vector2<float> m_drag; 
    Vector2<float> m_gravity;
    Vector2<float> m_inertiaMultiplier;
    Collider m_pushbox;
    float m_magnetLimit = 0.0f;
    float m_onSlopeWithAngle = 0.0f;

    bool m_isGrounded = true;
    bool m_noUpwardLanding = false;

    void velocityToInertia();
    Vector2<float> getPosOffest() const;
};

struct ComponentStaticCollider
{
    ComponentStaticCollider() = default;
    ComponentStaticCollider(const SlopeCollider &collider_);

    ComponentStaticCollider (const ComponentStaticCollider &rhs_) = delete;
    ComponentStaticCollider (ComponentStaticCollider &&rhs_) = default;
    ComponentStaticCollider &operator=(const ComponentStaticCollider &rhs_) = delete;
    ComponentStaticCollider &operator=(ComponentStaticCollider &&rhs_) = default;

    SlopeCollider m_collider;
};

struct ComponentObstacle
{
    int m_obstacleId;
};

struct ComponentTrigger
{
    Trigger m_trigger;
};


struct ComponentObstacleFallthrough
{
    ComponentObstacleFallthrough() = default;

    ComponentObstacleFallthrough (const ComponentObstacleFallthrough &rhs_) = delete;
    ComponentObstacleFallthrough (ComponentObstacleFallthrough &&rhs_) = default;
    ComponentObstacleFallthrough &operator=(const ComponentObstacleFallthrough &rhs_) = delete;
    ComponentObstacleFallthrough &operator=(ComponentObstacleFallthrough &&rhs_) = default;

    void setIgnoringObstacles();
    bool isIgnoringAllObstacles() const;
    bool touchedObstacleTop(int obstacleId_);
    bool touchedObstacleBottom(int obstacleId_);
    bool touchedObstacleSlope(int obstacleId_);
    bool touchedObstacleSide(int obstacleId_);
    bool checkIgnoringObstacle(int obstacleId_) const;

    FrameTimer<false> m_isIgnoringObstacles;
    std::set<int> m_ignoredObstacles;
};

struct ComponentAnimationRenderable
{
    ComponentAnimationRenderable() = default;

    ComponentAnimationRenderable (const ComponentAnimationRenderable &rhs_) = delete;
    ComponentAnimationRenderable (ComponentAnimationRenderable &&rhs_) = default;
    ComponentAnimationRenderable &operator=(const ComponentAnimationRenderable &rhs_) = delete;
    ComponentAnimationRenderable &operator=(ComponentAnimationRenderable &&rhs_) = default;

    std::map<int, std::unique_ptr<Animation>> m_animations;
    Animation *m_currentAnimation;
};

struct ComponentPlayerInput
{
    ComponentPlayerInput() = default;
    ComponentPlayerInput(std::unique_ptr<InputResolver> &&inputResolver_);

    ComponentPlayerInput (const ComponentPlayerInput &rhs_) = delete;
    ComponentPlayerInput (ComponentPlayerInput &&rhs_) = default;
    ComponentPlayerInput &operator=(const ComponentPlayerInput &rhs_) = delete;
    ComponentPlayerInput &operator=(ComponentPlayerInput &&rhs_) = default;

    std::unique_ptr<InputResolver> m_inputResolver;
};


#endif
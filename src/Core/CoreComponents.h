#ifndef CORE_COMPONENTS_H_
#define CORE_COMPONENTS_H_
#include "InputResolver.h"
#include "Vector2.h"
#include "FrameTimer.h"
#include "Renderer.h"
#include "Collider.h"
#include "AnimationManager.h"
#include "NavGraph.h"
#include "Trigger.h"
#include "FrameTimer.h"
#include <entt/entt.hpp>
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

struct ComponentParticlePrimitive
{
    SDL_RendererFlip m_flip;
    FrameTimer<false> m_lifetime;
    float angle = 0.0f;
    entt::entity m_tieTransform = entt::null;
};

struct ComponentSpawnLocation
{
    Vector2<float> m_location;
};

struct ComponentParticlePhysics
{
    Vector2<float> m_velocity;
    Vector2<float> m_inertia;
    Vector2<float> m_drag; 
    Vector2<float> m_gravity;
    Vector2<float> m_inertiaMultiplier;

    Vector2<float> getPosOffest() const;
    void applyDrag();
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
    Vector2<float> m_extraoffset;
    Vector2<float> m_drag; 
    Vector2<float> m_gravity;
    Vector2<float> m_inertiaMultiplier;
    Collider m_pushbox;
    bool m_onMovingPlatform = false;
    float m_magnetLimit = 0.0f;
    float m_onSlopeWithAngle = 0.0f;
    float m_lastSlopeAngle = 0.0f;
    uint32_t m_hitstopLeft = 0;

    // Always resetted to {0, 0} after physics iteration, not controlled or limited in any way
    Vector2<float> m_adjustOffset;

    entt::entity m_onGround = entt::null;
    entt::entity m_onWall = entt::null;

    bool m_noLanding = false;

    // Used to identify offset applied before collision resolution
    Vector2<float> m_calculatedOffset;

    // Used to calculate camera offset
    Vector2<float> m_appliedOffset;

    // Offset enforced by dynamic colliders, used for things like inertia
    Vector2<float> m_enforcedOffset;

    void convertToInertia(bool convertVelocity_, bool includeEnforced_);
    Vector2<float> getPosOffest() const;
};

struct PhysicalEvents
{
    bool m_touchedGround = false;
    bool m_lostGround = false;
};

struct ComponentStaticCollider
{
    ComponentStaticCollider() = default;
    ComponentStaticCollider(const Vector2<float> &pos_, const SlopeCollider &collider_, int obstacleId_);

    ComponentStaticCollider (const ComponentStaticCollider &rhs_) = delete;
    ComponentStaticCollider (ComponentStaticCollider &&rhs_) = default;
    ComponentStaticCollider &operator=(const ComponentStaticCollider &rhs_) = delete;
    ComponentStaticCollider &operator=(ComponentStaticCollider &&rhs_) = default;

    int m_obstacleId = 0;
    bool m_isEnabled = true;

    SlopeCollider m_proto;
    SlopeCollider m_resolved;
};

struct SwitchCollider
{
    uint32_t m_durationEnabled;
    uint32_t m_durationDisabled;
    FrameTimer<true> m_timer;
    bool m_isEnabled = true;

    bool updateTimer();
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
    bool setIgnoreObstacle(int obstacleId_);
    bool checkIgnoringObstacle(int obstacleId_) const;

    FrameTimer<false> m_isIgnoringObstacles;
    std::set<int> m_ignoredObstacles;
    std::set<int> m_overlappedObstacles;
};

class Flash
{
public:
    Flash(uint32_t duration_, uint32_t firstFrame = 0);
    virtual SDL_Color getFlashColor() const = 0;
    bool update();
    virtual std::unique_ptr<Flash> clone() const = 0;

protected:
    uint32_t m_fullDuration;
    uint32_t m_currentFrame;
};

class FlashLinear : public Flash
{
public:
    FlashLinear(uint32_t duration_, SDL_Color color_, uint32_t firstFrame_ = 0);
    virtual SDL_Color getFlashColor() const override;
    virtual std::unique_ptr<Flash> clone() const override;

private:
    SDL_Color m_flashColor;
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
    std::unique_ptr<Flash> m_flash;
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

struct ComponentDynamicCameraTarget
{
    ComponentDynamicCameraTarget() = default;

    ComponentDynamicCameraTarget (const ComponentDynamicCameraTarget &rhs_) = delete;
    ComponentDynamicCameraTarget (ComponentDynamicCameraTarget &&rhs_) = default;
    ComponentDynamicCameraTarget &operator=(const ComponentDynamicCameraTarget &rhs_) = delete;
    ComponentDynamicCameraTarget &operator=(ComponentDynamicCameraTarget &&rhs_) = default;

    Vector2<float> m_offset;
    Vector2<float> m_lookaheadSpeedSensitivity = {1.0f, 1.0f};
};

struct Navigatable
{
    Navigatable()
    {
        std::cout << "Nav created\n";
    }

    const Connection *m_currentOwnConnection = nullptr;
    bool m_checkIfGrounded = false;
    Traverse::TraitT m_validTraitsOwnLocation;
    Traverse::TraitT m_traverseTraits;
    float m_maxRange = 0.0f;

    std::shared_ptr<NavPath> m_currentPath;

    ~Navigatable()
    {
        std::cout << "Nav destroyed\n";
    }
};

template<size_t LAYER>
struct RenderLayer
{
    static_assert(LAYER < gamedata::global::renderLayerCount, "Layer does not exist");
};

struct MoveCollider2Points
{
    MoveCollider2Points(const Vector2<float> &point1_, const Vector2<float> &point2_, float duration_);

    Vector2<float> m_point1;
    Vector2<float> m_point2;
    float m_duration;
    FrameTimer<true> m_timer;
    bool m_toSecond;
};

Collider getColliderAt(const Collider &col_, const ComponentTransform &trans_);
bool checkCurrentHitstop(entt::registry &reg_, const entt::entity &idx_);

#endif
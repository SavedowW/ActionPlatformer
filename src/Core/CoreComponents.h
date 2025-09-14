#ifndef CORE_COMPONENTS_H_
#define CORE_COMPONENTS_H_
#include "Tileset.h"
#include "Vector2.hpp"
#include "FrameTimer.h"
#include "Collider.h"
#include "AnimationManager.h"
#include "NavGraph.h"
#include "NavSystem.h"
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

    ComponentTransform(const Vector2<int> &pos_, ORIENTATION orient_);

    ComponentTransform (const ComponentTransform &rhs_) = delete;
    ComponentTransform (ComponentTransform &&rhs_) = default;
    ComponentTransform &operator=(const ComponentTransform &rhs_) = delete;
    ComponentTransform &operator=(ComponentTransform &&rhs_) = default;
    
    Vector2<int> m_pos;
    ORIENTATION m_orientation = ORIENTATION::RIGHT;
};

struct ComponentParticlePrimitive
{
    SDL_FlipMode m_flip = SDL_FLIP_NONE;
    FrameTimer<false> m_lifetime{};
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

    void applyDrag();
    Vector2<int> claimOffset();
    Vector2<int> peekOffset() const;
    Vector2<float> peekRawOffset() const;

private:
    Vector2<float> m_velocityLeftover;
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
    unsigned int m_magnetLimit = 0;
    float m_onSlopeWithAngle = 0.0f;
    float m_lastSlopeAngle = 0.0f;
    uint32_t m_hitstopLeft = 0;

    const Vector2<float> *m_mulInsidePushbox = nullptr;

    entt::entity m_onGround = entt::null;
    entt::entity m_onWall = entt::null;

    bool m_noLanding = false;

    // Used to identify offset applied before collision resolution
    Vector2<int> m_calculatedOffset;

    // Used to calculate camera offset
    Vector2<int> m_appliedOffset;

    // Offset enforced by dynamic colliders, used for things like inertia
    Vector2<int> m_enforcedOffset;
    
    // Offset enforced by dynamic colliders that is already applied to the character (getting pushed by platforms, etc)
    Vector2<int> m_pushedOffset;
    
    Vector2<float> m_stateLeaveVelocityMultiplier;
    
    void convertToInertia(bool convertVelocity_, bool includeEnforced_);
    Vector2<int> claimOffset();
    Vector2<int> peekOffset() const;
    Vector2<float> peekRawOffset() const;

    Vector2<float> m_velocityLeftover;
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
    uint32_t m_durationEnabled = 0;
    uint32_t m_durationDisabled = 0;
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
    virtual uint8_t getFlashAlpha() const = 0;
    bool update();
    virtual std::unique_ptr<Flash> clone() const = 0;

    virtual ~Flash() = default;

protected:
    uint32_t m_fullDuration;
    uint32_t m_currentFrame;
};

class FlashDelayedLinear : public Flash
{
public:
    FlashDelayedLinear(uint32_t delayDuration, uint32_t fadeDuration_, uint32_t firstFrame_ = 0);
    virtual uint8_t getFlashAlpha() const override;
    virtual std::unique_ptr<Flash> clone() const override;

private:
    uint32_t m_delayDuration;
    uint32_t m_fadeDuration;
};

struct ComponentAnimationRenderable
{
    ComponentAnimationRenderable() = default;

    ComponentAnimationRenderable (const ComponentAnimationRenderable &rhs_) = delete;
    ComponentAnimationRenderable (ComponentAnimationRenderable &&rhs_) = default;
    ComponentAnimationRenderable &operator=(const ComponentAnimationRenderable &rhs_) = delete;
    ComponentAnimationRenderable &operator=(ComponentAnimationRenderable &&rhs_) = default;

    template<typename... Args>
    void loadAnimation(AnimationManager &animMgmt_, ResID id_, Args&&... args_)
    {
        m_animations.emplace(id_, Animation(animMgmt_, id_, std::forward<Args>(args_)...));
    }

    std::map<ResID, Animation> m_animations;
    Animation *m_currentAnimation;
    std::unique_ptr<Flash> m_flash;
    bool m_drawOutline = false;
};

struct ComponentDynamicCameraTarget
{
    ComponentDynamicCameraTarget() = default;

    ComponentDynamicCameraTarget (const ComponentDynamicCameraTarget &rhs_) = delete;
    ComponentDynamicCameraTarget (ComponentDynamicCameraTarget &&rhs_) = default;
    ComponentDynamicCameraTarget &operator=(const ComponentDynamicCameraTarget &rhs_) = delete;
    ComponentDynamicCameraTarget &operator=(ComponentDynamicCameraTarget &&rhs_) = default;

    Vector2<int> m_offset;
    Vector2<float> m_lookaheadSpeedSensitivity = {1.0f, 1.0f};
};

struct Navigatable
{
    // TODO: to iterator
    bool m_checkIfGrounded = false;

    // Overcomplication, but might be useful later
    // Traverse::TraitT m_validTraitsOwnLocation;

    Traverse::TraitT m_traverseTraits;
    float m_maxRange = 0.0f;
    float m_nodeTransitionRange = 0.0f;

    NavPath::Follower m_pathFollower;
};

enum class HUDPosRule : uint8_t
{
    POS_WORLD,
    REL_TRANSFORM
};

struct HUDPoint
{
    HUDPosRule m_posRule = HUDPosRule::POS_WORLD;
    Vector2<int> m_pos;
    int m_vOffset = 0;
};

struct RenderLayer
{
    RenderLayer(int depth_, bool visible_ = true) noexcept;
    RenderLayer(const RenderLayer&) noexcept;
    RenderLayer(RenderLayer&&) noexcept;
    RenderLayer &operator=(const RenderLayer&) = delete;
    RenderLayer &operator=(RenderLayer&&) noexcept;
    ~RenderLayer();

    int getDepth() const noexcept;
    bool isVisible() const noexcept;

    static bool m_dirtyOrder;

private:
    int m_depth;
    bool m_visible = true;
};

struct MoveCollider2Points
{
    MoveCollider2Points(const Vector2<float> &offset_);
    Vector2<float> m_point1;
    Vector2<float> m_point2;
    const Vector2<float> m_offset;
    FrameTimer<false> m_timer;
};

struct TilemapLayer
{
    TilemapLayer(const Vector2<int> &size_, const Vector2<float> &parallaxFactor_) :
    m_tiles(size_.y, std::vector<Tile>(size_.x, Tile{})),
    m_parallaxFactor(parallaxFactor_)
    {
    }

    std::vector<std::vector<Tile>> m_tiles;
    Vector2<float> m_parallaxFactor;
    Vector2<int> m_posOffset;
};

Collider getColliderAt(const Collider &col_, const ComponentTransform &trans_);
bool checkCurrentHitstop(entt::registry &reg_, const entt::entity &idx_);

#endif

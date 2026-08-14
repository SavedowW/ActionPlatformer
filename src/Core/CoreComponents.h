#pragma once
#include "Tileset.h"
#include "SlidingWindow.h"
#include "Vector2.hpp"
#include "FrameTimer.h"
#include "Collider.h"
#include "AnimationManager.h"
#include "NavGraph.h"
#include "NavSystem.h"
#include <entt/entt.hpp>
#include <set>
#include <map>
#include <memory>
#include <utility>

// These components are specific to the game and shouldnt be in the core but whatever

struct ComponentTransform
{
    ComponentTransform() = default;
    ComponentTransform(const Vector2<int> &pos_, Orientation orient_);
    
    Vector2<int> m_pos;
    Orientation m_orientation = Orientation::RIGHT;
};

struct ComponentSpawnLocation
{
    Vector2<int> location;
};

struct ComponentPhysical
{
    Vector2<float> velocity;
    Vector2<float> inertia;
    Vector2<float> extraoffset;
    Vector2<float> drag; 
    Vector2<float> gravity;
    Vector2<float> inertiaMultiplier = {1.0f, 1.0f};
    Collider pushbox;
    bool onMovingPlatform = false;
    unsigned int magnetLimit = 0;
    uint32_t hitstopLeft = 0;

    // TODO: find a way to do it without hurting parallelization
    //const Vector2<float> *m_mulInsidePushbox = nullptr;

    // Used to identify offset applied before collision resolution
    Vector2<int> calculatedOffset;

    // Used to calculate camera offset
    SlidingWindow<Vector2<int>, 10> appliedOffset;

    // Offset enforced by dynamic colliders, used for things like inertia
    Vector2<int> enforcedOffset;
    
    // Offset enforced by dynamic colliders that is already applied to the character (getting pushed by platforms, etc)
    Vector2<int> pushedOffset;
    
    Vector2<float> stateLeaveVelocityMultiplier;
    
    void convertToInertia(bool convertVelocity_, bool includeEnforced_);

    Vector2<int> claimOffset();
    Vector2<int> peekOffset() const;

    // Ignoring stored velocity
    Vector2<float> peekRawOffset() const;

    Vector2<float> velocityLeftover;
};

struct WorldPosition
{
    struct Ground
    {
        bool demand = true;
        float onSlopeWithAngle = 0.0f;
        entt::entity onGround = entt::null;
    } ground;

    struct Wall
    {
        bool demand = true;
        entt::entity leftWall = entt::null;
        entt::entity rightWall = entt::null;
    } wall;

    void reset();
};

/**
 *  All colliders that are obstacles allow clean passing through if an entity ends up inside. Some of them also cannot be interacted 
 */
enum class ObstacleType : uint8_t
{
    NONE,

    // Default for all moving platforms - becomes permeable if entity somehow gets inside, but behaves as regular collider otherwise
    MINIMAL,

    // Same as MINIMAL, but walls and bottom are also completely permeable and can be fallen through
    FLOOR
};

SERIALIZE_ENUM(ObstacleType, {
    ENUM_AUTO(ObstacleType, NONE),
    ENUM_AUTO(ObstacleType, MINIMAL),
    ENUM_AUTO(ObstacleType, FLOOR)
})

bool operator>(ObstacleType lhs_, ObstacleType rhs_) noexcept;
bool operator>=(ObstacleType lhs_, ObstacleType rhs_) noexcept;
bool operator<(ObstacleType lhs_, ObstacleType rhs_) noexcept;

struct ComponentStaticCollider
{
    ComponentStaticCollider() = default;
    ComponentStaticCollider(const Vector2<float> &pos_, const SlopeCollider &collider_, ObstacleType obstacleType_);

    ObstacleType obstacleType = ObstacleType::NONE;

    // TODO: to separate component
    bool m_isEnabled = true;

    SlopeCollider m_proto;
    SlopeCollider m_resolved;
};

struct ComponentObstacleFallthrough
{
    ComponentObstacleFallthrough() = default;

    ComponentObstacleFallthrough (const ComponentObstacleFallthrough &rhs_) = delete;
    ComponentObstacleFallthrough (ComponentObstacleFallthrough &&rhs_) noexcept = default;
    ComponentObstacleFallthrough &operator=(const ComponentObstacleFallthrough &rhs_) = delete;
    ComponentObstacleFallthrough &operator=(ComponentObstacleFallthrough &&rhs_) = default;

    // Down input - ignoring all
    void setIgnoringObstacles();

    // Down input
    bool isIgnoringAllObstacles() const;

    // Is it listed
    bool isIgnoringObstacle(entt::entity cid_) const;

    // Add to the list
    bool setIgnoreObstacle(entt::entity cid_);

    FrameTimer<false> m_isIgnoringObstacles;
    std::set<entt::entity> m_ignoredObstacles;
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
    uint8_t getFlashAlpha() const override;
    std::unique_ptr<Flash> clone() const override;

private:
    uint32_t m_delayDuration;
    uint32_t m_fadeDuration;
};

struct ComponentAnimationRenderable
{
    ComponentAnimationRenderable() = default;

    ComponentAnimationRenderable (const ComponentAnimationRenderable &rhs_) = delete;
    ComponentAnimationRenderable (ComponentAnimationRenderable &&rhs_) noexcept = default;
    ComponentAnimationRenderable &operator=(const ComponentAnimationRenderable &rhs_) = delete;
    ComponentAnimationRenderable &operator=(ComponentAnimationRenderable &&rhs_) = default;

    template<typename... Args>
    void loadAnimation(AnimationManager &animMgmt_, ResID id_, Args&&... args_)
    {
        m_animations.emplace(id_, Animation(animMgmt_, id_, std::forward<Args>(args_)...));
    }

    std::map<ResID, Animation> m_animations;
    Animation *m_currentAnimation = nullptr;
    std::unique_ptr<Flash> m_flash;
    bool m_drawOutline = false;
};

struct ComponentDynamicCameraTarget
{
    Vector2<int> currentOffset;
    Vector2<float> lookaheadSpeedSensitivity = {1.0f, 1.0f};
};

struct Navigatable
{
    // TODO: to iterator
    bool m_checkIfGrounded = false;

    // Overcomplication, but might be useful later
    // Traverse::TraitT m_validTraitsOwnLocation;

    Traverse::TraitT m_traverseTraits = 0;
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
    RenderLayer &operator=(const RenderLayer&) noexcept;
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
    MoveCollider2Points(const Vector2<int> &offset_);
    Vector2<int> m_point1;
    Vector2<int> m_point2;
    const Vector2<int> m_offset;
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

struct ComponentParticlePrimitive
{
    FrameTimer<false> lifetime;
    float angle = 0.0f;
    entt::entity tieTransform = entt::null;
};

struct ComponentChildParticles
{
    std::vector<entt::entity> destroyOnStateChange;
};

Collider getColliderAt(const Collider &col_, const ComponentTransform &trans_);
bool checkCurrentHitstop(entt::registry &reg_, const entt::entity &idx_);

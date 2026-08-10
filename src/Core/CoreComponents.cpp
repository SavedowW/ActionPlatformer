#include "CoreComponents.h"
#include "SlidingWindow.h"
#include "StaticMapping.hpp"

ComponentTransform::ComponentTransform(const Vector2<int> &pos_, ORIENTATION orient_) :
    m_pos{pos_}, 
    m_orientation{orient_}
{
}

void ComponentPhysical::convertToInertia(bool convertVelocity_, bool includeEnforced_)
{
    if (convertVelocity_)
    {
        inertia += velocity;
        velocity = {0.0f, 0.0f};
    }
    
    if (includeEnforced_ || !onMovingPlatform)
    {
        //std::cout << "Adding enforced velocity: " << enforcedOffset << std::endl;
        inertia += enforcedOffset;
    }
}

Vector2<int> ComponentPhysical::claimOffset()
{
    auto offset = velocity + inertia.mulComponents(inertiaMultiplier) + extraoffset + velocityLeftover;
    Vector2<int> iOffset = offset;
    velocityLeftover = offset - iOffset;
    //std::cout << "{" << offset << "} - {" << iOffset << "} = " << velocityLeftover << std::endl;
    return iOffset;
}

Vector2<int> ComponentPhysical::peekOffset() const
{
    return velocity + inertia.mulComponents(inertiaMultiplier) + extraoffset + velocityLeftover;
}

Vector2<float> ComponentPhysical::peekRawOffset() const
{
    return velocity + inertia.mulComponents(inertiaMultiplier) + extraoffset;
}


void WorldPosition::reset()
{
    ground.onSlopeWithAngle = 0.0f;
    ground.onGround = entt::null;

    wall.leftWall = entt::null;
    wall.rightWall = entt::null;
}


void ComponentObstacleFallthrough::setIgnoringObstacles()
{
    m_isIgnoringObstacles.begin(5);
}

bool ComponentObstacleFallthrough::isIgnoringAllObstacles() const
{
    return m_isIgnoringObstacles.isActive();
}

bool ComponentObstacleFallthrough::checkIgnoringObstacle(int obstacleId_) const
{
    return m_isIgnoringObstacles.isActive() || m_ignoredObstacles.contains(obstacleId_);
}

bool ComponentObstacleFallthrough::setIgnoreObstacle(int obstacleId_)
{
    m_ignoredObstacles.insert(obstacleId_);
    return false;
}

ComponentStaticCollider::ComponentStaticCollider(const Vector2<float> &pos_, const SlopeCollider &collider_, int obstacleId_) :
    m_obstacleId(obstacleId_),
    m_proto(collider_),
    m_resolved(collider_.movedBy(pos_))
{

}

bool SwitchCollider::updateTimer()
{
    if (m_timer.update())
    {
        if (m_isEnabled)
            m_timer.begin(m_durationDisabled);
        else
            m_timer.begin(m_durationEnabled);

        m_isEnabled = !m_isEnabled;
    }

    return m_isEnabled;
}

Collider getColliderAt(const Collider &col_, const ComponentTransform &trans_)
{
    if (trans_.m_orientation == ORIENTATION::LEFT)
        return Collider{.m_topLeft={trans_.m_pos.x - col_.m_topLeft.x - col_.m_size.x + 1, trans_.m_pos.y + col_.m_topLeft.y + 1}, .m_size=col_.m_size};

    return Collider{.m_topLeft=trans_.m_pos + col_.m_topLeft + Vector2{1, 1}, .m_size=col_.m_size};
}

bool checkCurrentHitstop(entt::registry &reg_, const entt::entity &idx_)
{
    auto *phys = reg_.try_get<ComponentPhysical>(idx_);

    return phys && phys->hitstopLeft;
}

FlashDelayedLinear::FlashDelayedLinear(uint32_t delayDuration_, uint32_t fadeDuration_, uint32_t firstFrame_) :
    Flash(delayDuration_ + fadeDuration_, firstFrame_),
    m_delayDuration(delayDuration_),
    m_fadeDuration(fadeDuration_)
{
}

uint8_t FlashDelayedLinear::getFlashAlpha() const
{
    if (m_currentFrame < m_delayDuration)
        return 255;

    return static_cast<uint8_t>(255 * (1.0f - static_cast<float>(m_currentFrame - m_delayDuration) / static_cast<float>(m_fadeDuration)));
}

std::unique_ptr<Flash> FlashDelayedLinear::clone() const
{
    return std::make_unique<FlashDelayedLinear>(m_delayDuration, m_fadeDuration, m_currentFrame);
}

Flash::Flash(uint32_t duration_, uint32_t firstFrame) :
    m_fullDuration(duration_),
    m_currentFrame(firstFrame)
{
}

bool Flash::update()
{
    if (m_currentFrame >= m_fullDuration)
        return true;

    return ++m_currentFrame >= m_fullDuration;
}

RenderLayer::RenderLayer(int depth_, bool visible_) noexcept :
    m_depth(depth_),
    m_visible(visible_)
{
    m_dirtyOrder = true;
}

RenderLayer::RenderLayer(const RenderLayer &rhs_) noexcept :
    m_depth(rhs_.m_depth),
    m_visible(rhs_.m_visible)
{
    m_dirtyOrder = true;
}

RenderLayer::RenderLayer(RenderLayer &&rhs_) noexcept :
    m_depth(rhs_.m_depth),
    m_visible(rhs_.m_visible)
{
    m_dirtyOrder = true;
}

RenderLayer &RenderLayer::operator=(const RenderLayer &rhs_) noexcept
{
    m_depth = rhs_.m_depth;
    m_visible = rhs_.m_visible;
    m_dirtyOrder = true;

    return *this;
}

RenderLayer &RenderLayer::operator=(RenderLayer &&rhs_) noexcept
{
    m_depth = rhs_.m_depth;
    m_visible = rhs_.m_visible;
    m_dirtyOrder = true;

    return *this;
}

int RenderLayer::getDepth() const noexcept
{
    return m_depth;
}

bool RenderLayer::isVisible() const noexcept
{
    return m_visible;
}

RenderLayer::~RenderLayer()
{
    m_dirtyOrder = true;
}

bool RenderLayer::m_dirtyOrder = false;

MoveCollider2Points::MoveCollider2Points(const Vector2<int> &offset_) :
    m_offset(offset_)
{
}

T_NAME_AUTO(ComponentTransform);
T_NAME_AUTO(const ComponentTransform);
T_NAME_AUTO(ComponentParticlePrimitive);
T_NAME_AUTO(ComponentSpawnLocation);
T_NAME_AUTO(ComponentPhysical);
T_NAME_AUTO(const ComponentPhysical);
T_NAME_AUTO(WorldPosition);
T_NAME_AUTO(ComponentStaticCollider);
T_NAME_AUTO(SwitchCollider);
T_NAME_AUTO(ComponentObstacleFallthrough);
T_NAME_AUTO(ComponentAnimationRenderable);
T_NAME_AUTO(ComponentDynamicCameraTarget);
T_NAME_AUTO(Navigatable);
T_NAME_AUTO(HUDPoint);
T_NAME_AUTO(RenderLayer);
T_NAME_AUTO(MoveCollider2Points);
T_NAME_AUTO(TilemapLayer);
T_NAME_AUTO(Flash);
T_NAME_AUTO(FlashDelayedLinear);
T_NAME_AUTO(HUDPosRule);

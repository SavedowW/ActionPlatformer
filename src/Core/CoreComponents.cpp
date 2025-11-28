#include "CoreComponents.h"
#include "StaticMapping.hpp"
#include "Application.h"
#include "Timer.h"

ComponentTransform::ComponentTransform(const Vector2<int> &pos_, ORIENTATION orient_) :
    m_pos(pos_), m_orientation(orient_)
{
}

void ComponentPhysical::convertToInertia(bool convertVelocity_, bool includeEnforced_)
{
    if (convertVelocity_)
    {
        m_inertia += m_velocity;
        m_velocity = {0.0f, 0.0f};
    }
    
    if (includeEnforced_ || !m_onMovingPlatform)
    {
        //std::cout << "Adding enforced velocity: " << m_enforcedOffset << std::endl;
        m_inertia += m_enforcedOffset;
    }
}

Vector2<int> ComponentPhysical::claimOffset(const double &partOfSecond_)
{
    //if (m_velocity.x >= 2.5f)
    //    std::cout << m_velocity << std::endl;
    const auto offset = (m_velocity + m_inertia.mulComponents(m_inertiaMultiplier) + m_extraoffset) * partOfSecond_ + m_velocityLeftover;
    const Vector2<int> iOffset = offset;
    m_velocityLeftover = offset - iOffset;
    std::cout << "{" << offset << "} - {" << iOffset << "} = " << m_velocityLeftover << std::endl;
    return iOffset;
}

Vector2<int> ComponentPhysical::peekOffset() const
{
    return m_velocity + m_inertia.mulComponents(m_inertiaMultiplier) + m_extraoffset + m_velocityLeftover;
}

Vector2<float> ComponentPhysical::peekRawOffset() const
{
    return m_velocity + m_inertia.mulComponents(m_inertiaMultiplier) + m_extraoffset + m_velocityLeftover;
}

void ComponentObstacleFallthrough::setIgnoringObstacles()
{
    m_isIgnoringObstacles.begin(Time::fromFrames(5));
}

bool ComponentObstacleFallthrough::isIgnoringAllObstacles() const
{
    return m_isIgnoringObstacles.isActive();
}

// true if not ignored
bool ComponentObstacleFallthrough::touchedObstacleTop(int obstacleId_)
{
    if (m_isIgnoringObstacles.isActive())
    {
        m_ignoredObstacles.insert(obstacleId_);
        return false;
    }

    return !m_ignoredObstacles.contains(obstacleId_);
}

bool ComponentObstacleFallthrough::touchedObstacleSlope(int obstacleId_)
{
    if (m_isIgnoringObstacles.isActive())
    {
        m_ignoredObstacles.insert(obstacleId_);
        return false;
    }
    return !m_ignoredObstacles.contains(obstacleId_);
}

bool ComponentObstacleFallthrough::checkIgnoringObstacle(int obstacleId_) const
{
    return m_isIgnoringObstacles.isActive() || m_ignoredObstacles.contains(obstacleId_);
}

bool ComponentObstacleFallthrough::touchedObstacleBottom(int obstacleId_)
{
    m_ignoredObstacles.insert(obstacleId_);
    return false;
}

bool ComponentObstacleFallthrough::touchedObstacleSide(int obstacleId_)
{
    m_ignoredObstacles.insert(obstacleId_);
    return false;
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
    if (m_timer.update(Application::instance().timestep.getFrameDuration()))
    {
        if (m_isEnabled)
            m_timer.begin(m_durationDisabled);
        else
            m_timer.begin(m_durationEnabled);

        m_isEnabled = !m_isEnabled;
    }

    return m_isEnabled;
}

void ComponentParticlePhysics::applyDrag(const double &partOfSecond_)
{
    if (m_inertia.x != 0)
    {
        auto absInertia = abs(m_inertia.x);
        auto inertiaSign = utils::signof(m_inertia.x / abs(m_inertia.x));
        absInertia = std::max(absInertia - m_drag.x * partOfSecond_, 0.0);
        m_inertia.x = inertiaSign * absInertia;
    }

    if (m_inertia.y != 0)
    {
        auto absInertia = abs(m_inertia.y);
        auto inertiaSign = utils::signof(m_inertia.y / abs(m_inertia.y));
        absInertia = std::max(absInertia - m_drag.y * partOfSecond_, 0.0);
        m_inertia.y = inertiaSign * absInertia;
    }
}

Vector2<int> ComponentParticlePhysics::claimOffset(const double &partOfSecond_)
{
    const auto offset = (m_velocity + m_inertia.mulComponents(m_inertiaMultiplier) * partOfSecond_) * partOfSecond_;
    Vector2<int> iOffset = offset;
    m_velocityLeftover = offset - iOffset;
    return iOffset;
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

    return phys && phys->m_hitstopLeft;
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

    return static_cast<uint8_t>(255 * (1.0f - static_cast<float>(m_currentFrame - m_delayDuration) / m_fadeDuration));
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

RenderLayer &RenderLayer::operator=(RenderLayer &&rhs_) noexcept
{
    m_depth = rhs_.m_depth;
    m_visible = rhs_.m_visible;

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

MoveCollider2Points::MoveCollider2Points(const Vector2<float> &offset_) :
    m_offset(offset_)
{
}

T_NAME_AUTO(ComponentTransform);
T_NAME_AUTO(const ComponentTransform);
T_NAME_AUTO(ComponentParticlePrimitive);
T_NAME_AUTO(ComponentSpawnLocation);
T_NAME_AUTO(ComponentParticlePhysics);
T_NAME_AUTO(ComponentPhysical);
T_NAME_AUTO(const ComponentPhysical);
T_NAME_AUTO(PhysicalEvents);
T_NAME_AUTO(ComponentStaticCollider);
T_NAME_AUTO(SwitchCollider);
T_NAME_AUTO(ComponentTrigger);
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

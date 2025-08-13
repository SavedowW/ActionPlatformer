#include "CoreComponents.h"
#include <vector>

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

Vector2<int> ComponentPhysical::claimOffset()
{
    auto offset = m_velocity + m_inertia.mulComponents(m_inertiaMultiplier) + m_extraoffset + m_velocityLeftover;
    Vector2<int> iOffset = offset;
    m_velocityLeftover = offset - iOffset;
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
    m_isIgnoringObstacles.begin(5);
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

ComponentPlayerInput::ComponentPlayerInput(std::unique_ptr<InputResolver> &&inputResolver_) :
    m_inputResolver(std::move(inputResolver_))
{
}

ComponentStaticCollider::ComponentStaticCollider(const Vector2<float> &pos_, const SlopeCollider &collider_, int obstacleId_) :
    m_obstacleId(obstacleId_),
    m_proto(collider_),
    m_resolved(pos_, collider_.m_size, collider_.m_topAngleCoef)
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

void ComponentParticlePhysics::applyDrag()
{
    if (m_inertia.x != 0)
    {
        auto absInertia = abs(m_inertia.x);
        auto m_inertiaSign = utils::signof(m_inertia.x / abs(m_inertia.x));
        absInertia = std::max(absInertia - m_drag.x, 0.0f);
        m_inertia.x = m_inertiaSign * absInertia;
    }

    if (m_inertia.y != 0)
    {
        auto absInertia = abs(m_inertia.y);
        auto m_inertiaSign = utils::signof(m_inertia.y / abs(m_inertia.y));
        absInertia = std::max(absInertia - m_drag.y, 0.0f);
        m_inertia.y = m_inertiaSign * absInertia;
    }
}

Vector2<int> ComponentParticlePhysics::claimOffset()
{
    auto offset = m_velocity + m_inertia.mulComponents(m_inertiaMultiplier);
    Vector2<int> iOffset = offset;
    m_velocityLeftover = offset - iOffset;
    return iOffset;
}

Vector2<float> ComponentParticlePhysics::peekRawOffset() const
{
    return m_velocity + m_inertia.mulComponents(m_inertiaMultiplier);
}

Collider getColliderAt(const Collider &col_, const ComponentTransform &trans_)
{
    if (trans_.m_orientation == ORIENTATION::LEFT)
        return Collider{{trans_.m_pos.x - col_.m_topLeft.x - col_.m_size.x + 1, trans_.m_pos.y + col_.m_topLeft.y + 1}, col_.m_size};
    else
        return Collider{trans_.m_pos + col_.m_topLeft + Vector2{1, 1}, col_.m_size};
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
    else
        return static_cast<uint8_t>(255 * (1.0f - static_cast<float>(m_currentFrame - m_delayDuration) / m_fadeDuration));
}

std::unique_ptr<Flash> FlashDelayedLinear::clone() const
{
    return std::unique_ptr<Flash>(new FlashDelayedLinear(m_delayDuration, m_fadeDuration, m_currentFrame));
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

RenderLayer::RenderLayer(int depth_) :
    m_depth(depth_)
{
    m_dirtyOrder = true;
}

RenderLayer::RenderLayer(const RenderLayer &rhs_) :
    m_depth(rhs_.m_depth),
    m_visible(rhs_.m_visible)
{
    m_dirtyOrder = true;
}

RenderLayer::RenderLayer(RenderLayer &&rhs_) :
    m_depth(rhs_.m_depth),
    m_visible(rhs_.m_visible)
{
    m_dirtyOrder = true;
}

RenderLayer &RenderLayer::operator=(RenderLayer &&rhs_)
{
    m_depth = rhs_.m_depth;
    m_visible = rhs_.m_visible;

    return *this;
}

RenderLayer::~RenderLayer()
{
    m_dirtyOrder = true;
}

bool RenderLayer::m_dirtyOrder = false;

MoveCollider2Points::MoveCollider2Points(const Vector2<float> offset_) :
    m_offset(offset_)
{
}

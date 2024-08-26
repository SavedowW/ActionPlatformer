#include "CoreComponents.h"

ComponentTransform::ComponentTransform(const Vector2<float> &pos_, ORIENTATION orient_) :
    m_pos(pos_), m_orientation(orient_)
{
}

void ComponentPhysical::velocityToInertia()
{
    m_inertia += m_velocity;
    m_velocity = {0.0f, 0.0f};
}

Vector2<float> ComponentPhysical::getPosOffest() const
{
    return m_velocity + m_inertia.mulComponents(m_inertiaMultiplier);
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

ComponentStaticCollider::ComponentStaticCollider(const SlopeCollider &collider_, int obstacleId_) :
    m_collider(collider_),
    m_obstacleId(obstacleId_)
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

Vector2<float> ComponentParticlePhysics::getPosOffest() const
{
    return m_velocity + m_inertia.mulComponents(m_inertiaMultiplier);
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

MoveCollider2Points::MoveCollider2Points(const Vector2<float> &point1_, const Vector2<float> &point2_, float duration_) :
    m_point1(point1_),
    m_point2(point2_),
    m_duration(duration_),
    m_toSecond(true)
{
    m_timer.begin(duration_);
}

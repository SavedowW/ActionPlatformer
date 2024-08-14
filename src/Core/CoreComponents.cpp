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

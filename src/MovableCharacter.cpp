#include "MovableCharacter.h"

MovableCharacter::MovableCharacter(Application &application_, const CollisionArea &cldArea_) :
    Object(application_),
    m_collisionArea(cldArea_),
    m_onSlopeWithAngle(0)
{

}

void MovableCharacter::cleanIgnoredObstacles()
{
    auto touched = m_collisionArea.getPlayerTouchingObstacles(getPushbox());
    std::set<int> res;
    std::set_intersection(
        m_ignoredObstacles.begin(), m_ignoredObstacles.end(),
        touched.begin(), touched.end(),
        std::inserter(res, res.begin()));

    m_ignoredObstacles = res;
}

bool MovableCharacter::touchedObstacleTop(int obstacleId_)
{
    return !m_ignoredObstacles.contains(obstacleId_);
}

bool MovableCharacter::touchedObstacleBottom(int obstacleId_)
{
    m_ignoredObstacles.insert(obstacleId_);
    return false;
}

bool MovableCharacter::touchedObstacleSlope(int obstacleId_)
{
    return !m_ignoredObstacles.contains(obstacleId_);
}

bool MovableCharacter::touchedObstacleSide(int obstacleId_)
{
    m_ignoredObstacles.insert(obstacleId_);
    return false;
}

bool MovableCharacter::checkIgnoringObstacle(int obstacleId_) const
{
    return m_ignoredObstacles.contains(obstacleId_);
}

void MovableCharacter::setSlopeAngle(float angle_)
{
    m_onSlopeWithAngle = angle_;
}

int MovableCharacter::getSlopeAngle() const
{
    return m_onSlopeWithAngle;
}

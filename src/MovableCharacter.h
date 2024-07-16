#ifndef MOVABLE_CHARACTER_H_
#define MOVABLE_CHARACTER_H_
#include "Object.h"
#include "CollisionArea.h"

class MovableCharacter : public Object
{
public:
    MovableCharacter(Application &application_, const CollisionArea &cldArea_);

    virtual bool isIgnoringAllObstacles() = 0;
    virtual void cleanIgnoredObstacles();
    virtual bool touchedObstacleTop(int obstacleId_);
    virtual bool touchedObstacleBottom(int obstacleId_);
    virtual bool touchedObstacleSlope(int obstacleId_);
    virtual bool touchedObstacleSide(int obstacleId_);
    virtual bool checkIgnoringObstacle(int obstacleId_) const;

    void setSlopeAngle(float angle_);
    int getSlopeAngle() const;

    virtual void onTouchedGround() = 0;
    virtual void onLostGround() = 0;
    virtual bool attemptResetGround() = 0;

protected:
    std::set<int> m_ignoredObstacles;
    const CollisionArea &m_collisionArea;
    float m_onSlopeWithAngle;

};

#endif
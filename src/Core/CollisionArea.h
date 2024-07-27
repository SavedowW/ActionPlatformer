#ifndef COLLISION_AREA_H_
#define COLLISION_AREA_H_
#include "Collider.h"
#include "GameData.h"
#include "Trigger.h"
#include <set>

/*
    TODO: dynamic collider views left-to-right and right-to-left
    Otherwise there are ambiguous situations with wrong result
    Exmaple:
    C
    CC <= PPPP
    CCC   PPPP
    CCCC  PPPP
        C PPPP
        CC
        CCC
        CCCC

    Result depends on the order in which colliders are proceeded
*/

struct CollisionArea
{
    bool getHighestVerticalMagnetCoord(const Collider &cld_, float &coord_, const std::set<int> ignoredObstacles_) const;
    void finalize();
    bool isAreaFree(const Collider &cld_, bool considerObstacles_);
    const Trigger *getOverlappedTrigger(const Collider &cld_, Trigger::Tag tag_) const;

    std::set<int> getPlayerTouchingObstacles(const Collider &playerPb_) const;

    std::vector<SlopeCollider> m_staticCollisionMap;
    std::vector<int> m_obstacles;

    std::vector<Trigger> m_triggers;

    int m_nextSlope = 0;
};

#endif
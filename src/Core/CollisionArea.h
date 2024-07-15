#ifndef COLLISION_AREA_H_
#define COLLISION_AREA_H_
#include "Collider.h"
#include "GameData.h"
#include "Trigger.h"
#include <set>

class PlayableCharacter;

template<typename T1, typename T2>
inline Collider getColliderForTileRange(Vector2<T1> pos_, Vector2<T2> size_)
{
    return {gamedata::global::tileSize.mulComponents(pos_), gamedata::global::tileSize.mulComponents(size_)};
}

template<typename T1, typename T2>
inline SlopeCollider getColliderForTileRange(Vector2<T1> pos_, Vector2<T2> size_, float angle_, int obstacleId_ = 0)
{
    return {gamedata::global::tileSize.mulComponents(pos_), gamedata::global::tileSize.mulComponents(size_), angle_, obstacleId_};
}

template<typename T>
inline Vector2<float> getTileCenter(Vector2<T> pos_)
{
    return gamedata::global::tileSize.mulComponents(pos_) - gamedata::global::tileSize / 2;
}

template<typename T>
inline Vector2<float> getTilePos(Vector2<T> pos_)
{
    return gamedata::global::tileSize.mulComponents(pos_);
}

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
    void addStaticCollider(const SlopeCollider &cld_);
    bool getHighestVerticalMagnetCoord(const Collider &cld_, float &coord_, const PlayableCharacter &char_) const;
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
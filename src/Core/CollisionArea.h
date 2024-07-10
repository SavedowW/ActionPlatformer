#ifndef COLLISION_AREA_H_
#define COLLISION_AREA_H_
#include "Collider.h"
#include "GameData.h"

template<typename T1, typename T2>
inline Collider getColliderForTileRange(Vector2<T1> pos_, Vector2<T2> size_)
{
    return {gamedata::global::tileSize.mulComponents(pos_), gamedata::global::tileSize.mulComponents(size_)};
}

template<typename T1, typename T2, bool IS_OBSTACLE = false>
inline SlopeCollider getColliderForTileRange(Vector2<T1> pos_, Vector2<T2> size_, float angle_)
{
    return {gamedata::global::tileSize.mulComponents(pos_), gamedata::global::tileSize.mulComponents(size_), angle_, IS_OBSTACLE};
}

template<typename T>
inline Vector2<float> getTileCenter(Vector2<T> pos_)
{
    return gamedata::global::tileSize.mulComponents(pos_) - gamedata::global::tileSize / 2;
}

struct CollisionArea
{
    void addStaticCollider(const SlopeCollider &cld_);
    bool getHighestVerticalMagnetCoord(const Collider &cld_, float &coord_, bool isFallingThrough_) const;

    bool checkPlayerTouchingObstacles(const Collider &playerPb_) const;

    std::vector<SlopeCollider> m_staticCollisionMap;
    std::vector<int> m_obstacles;
};

#endif
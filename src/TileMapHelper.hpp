#ifndef TILE_MAP_HELPER_H_
#define TILE_MAP_HELPER_H_
#include "Collider.h"
#include "Trigger.h"
#include "GameData.h"

template<typename T1, typename T2>
inline Collider getColliderForTileRange(Vector2<T1> pos_, Vector2<T2> size_)
{
    return {gamedata::global::tileSize.mulComponents(pos_), gamedata::global::tileSize.mulComponents(size_)};
}

template<typename T1, typename T2>
inline SlopeCollider getColliderForTileRange(Vector2<T1> pos_, Vector2<T2> size_, float angle_)
{
    return {gamedata::global::tileSize.mulComponents(pos_), gamedata::global::tileSize.mulComponents(size_), angle_};
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

Trigger createTrigger(Vector2<int> cellTl_, Vector2<int> cellBr_, Trigger::Tag tag_);

#endif
#include "TileMapHelper.hpp"

Trigger createTrigger(Vector2<int> cellTl_, Vector2<int> cellBr_, Trigger::Tag tag_)
{
    const float topOffset = 20.0f;
    const float botOffset = 20.0f;

    float x = 0;
    if ((tag_ & Trigger::Tag::LEFT) == Trigger::Tag::LEFT)
        x = (cellTl_.x) * gamedata::global::tileSize.x - 1;
    else
        x = (cellTl_.x + 1) * gamedata::global::tileSize.x;

    Trigger trgarea{x, cellTl_.y * gamedata::global::tileSize.y + topOffset, 1.0f, (cellBr_.y - cellTl_.y + 1) * gamedata::global::tileSize.y - topOffset - botOffset};

    return trgarea;

}
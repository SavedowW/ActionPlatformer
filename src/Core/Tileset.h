#ifndef TILESET_H_
#define TILESET_H_

#include "Application.h"

struct TileView
{
    SDL_Texture *m_tex;
    SDL_Rect m_src;
};

struct Tile
{
    TileView &m_tile;
    Vector2<float> m_pos;
};

class Tileset
{
public:
    Tileset(Application *application_);
    void load(const std::string &spritesheet_);
    Tile getTile(const Vector2<float> pos_, unsigned gid_);

private:
    TextureManager &m_texManager;
    Texture_t m_tex;
    std::vector<TileView> m_tiles;
    Vector2<int> m_size;

};

#endif
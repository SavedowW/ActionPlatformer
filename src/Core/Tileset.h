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
    SDL_RendererFlip m_flip;
};

class Tileset
{
public:
    Tileset(Application *application_);
    void load(const std::string &spritesheet_);
    Tile getTile(const Vector2<float> pos_, unsigned gid_);

private:
    SDL_RendererFlip flagsToFlip(unsigned guid_) const;

    const unsigned FLIPPED_HORIZONTALLY_FLAG  = 0x80000000;
    const unsigned FLIPPED_VERTICALLY_FLAG    = 0x40000000;
    const unsigned FLIPPED_DIAGONALLY_FLAG    = 0x20000000;
    const unsigned ROTATED_HEXAGONAL_120_FLAG = 0x10000000;    

    TextureManager &m_texManager;
    Texture_t m_tex;
    std::vector<TileView> m_tiles;
    Vector2<int> m_size;

};

#endif
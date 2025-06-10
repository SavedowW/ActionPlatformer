#ifndef TILESET_H_
#define TILESET_H_

#include "TextureManager.h"
#include "TimelineProperty.hpp"

struct TileView
{
    unsigned int m_tex;
    Vector2<int> m_tilePos; // Position of TL corner on the original sheet in pixels
};

struct Tile
{
    TileView *m_tile = nullptr;
    SDL_RendererFlip m_flip = SDL_FLIP_NONE;
};

class Tileset
{
public:
    Tileset(TextureManager &texMan_, uint32_t firstgid_);
    void load(const std::string &spritesheet_);
    TileView *getView(uint32_t id_);

private:
    uint32_t m_firstgid;
    TextureManager &m_texManager;
    std::shared_ptr<Texture> m_tex;
    std::vector<TileView> m_tiles;

    // Size in tiles
    Vector2<int> m_size;

};

class TilesetBase
{
public:
    TilesetBase(TextureManager &texMan_);
    void addTileset(const std::string &spritesheet_, uint32_t firstgid_);
    Tile getTile(uint32_t gid_);

    static SDL_RendererFlip flagsToFlip(uint32_t gid_);

private:

    static const unsigned FLIPPED_HORIZONTALLY_FLAG;
    static const unsigned FLIPPED_VERTICALLY_FLAG;
    static const unsigned FLIPPED_DIAGONALLY_FLAG;
    static const unsigned ROTATED_HEXAGONAL_120_FLAG;   

    std::vector<Tileset> m_tilesets;
    TimelineProperty<size_t> m_tilesetMapping;
    TextureManager m_texManager;
};

#endif

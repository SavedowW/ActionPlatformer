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
    TileView *m_tile = nullptr;
    SDL_RendererFlip m_flip = SDL_FLIP_NONE;
};

class Tileset
{
public:
    Tileset(Application &app_, uint32_t firstgid_);
    void load(const std::string &spritesheet_);
    TileView *getView(uint32_t id_);

private:
    uint32_t m_firstgid;
    TextureManager &m_texManager;
    Texture_t m_tex;
    std::vector<TileView> m_tiles;
    Vector2<int> m_size;

};

class TilesetBase
{
public:
    TilesetBase(Application &app_);
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
    Application &m_app;
};

#endif
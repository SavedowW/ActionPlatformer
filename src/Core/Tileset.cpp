#include "Tileset.h"

const unsigned TilesetBase::FLIPPED_HORIZONTALLY_FLAG  = 0x80000000;
const unsigned TilesetBase::FLIPPED_VERTICALLY_FLAG    = 0x40000000;
const unsigned TilesetBase::FLIPPED_DIAGONALLY_FLAG    = 0x20000000;
const unsigned TilesetBase::ROTATED_HEXAGONAL_120_FLAG = 0x10000000;   

Tileset::Tileset(Application &app_, uint32_t firstgid_) :
    m_texManager(*app_.getTextureManager()),
    m_firstgid(firstgid_)
{
}

void Tileset::load(const std::string &spritesheet_)
{
    m_tiles.clear();

    m_tex = m_texManager.getTexture(m_texManager.getTexID(spritesheet_));

    for (m_size.y = 0; m_size.y < m_tex->m_h / gamedata::global::tileSize.y; ++m_size.y)
    {
        for (m_size.x = 0; m_size.x < m_tex->m_w / gamedata::global::tileSize.x; ++m_size.x)
        {
            m_tiles.push_back({m_tex->getSprite(), SDL_Rect{m_size.x * (int)gamedata::global::tileSize.x, m_size.y * (int)gamedata::global::tileSize.y, (int)gamedata::global::tileSize.x, (int)gamedata::global::tileSize.y}});
        }
    }
}

TileView *Tileset::getView(uint32_t id_)
{
    return &m_tiles[id_ - m_firstgid];
}

TilesetBase::TilesetBase(Application &app_) :
    m_app(app_)
{
}

SDL_RendererFlip TilesetBase::flagsToFlip(uint32_t gid_)
{
    SDL_RendererFlip res = SDL_FLIP_NONE;

    if (gid_ & FLIPPED_HORIZONTALLY_FLAG)
        res = SDL_RendererFlip(res | SDL_FLIP_HORIZONTAL);

    if (gid_ & FLIPPED_VERTICALLY_FLAG)
        res = SDL_RendererFlip(res | SDL_FLIP_VERTICAL);

    return res;
}

void TilesetBase::addTileset(const std::string &spritesheet_, uint32_t firstgid_)
{
    m_tilesets.emplace_back(m_app, firstgid_);
    m_tilesets.back().load(spritesheet_);

    m_tilesetMapping.addPropertyValue(firstgid_, m_tilesets.size() - 1);
}

Tile TilesetBase::getTile(uint32_t gid_)
{
    auto flags = flagsToFlip(gid_);
    gid_ &= ~(FLIPPED_HORIZONTALLY_FLAG |
        FLIPPED_VERTICALLY_FLAG |
        FLIPPED_DIAGONALLY_FLAG |
        ROTATED_HEXAGONAL_120_FLAG);

    int tilesetId = m_tilesetMapping[gid_];
    return {m_tilesets[tilesetId].getView(gid_), flags};
}

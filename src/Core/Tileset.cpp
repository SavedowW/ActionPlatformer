#include "Tileset.h"
#include "GameData.h"

const unsigned TilesetBase::FLIPPED_HORIZONTALLY_FLAG  = 0x80000000;
const unsigned TilesetBase::FLIPPED_VERTICALLY_FLAG    = 0x40000000;
const unsigned TilesetBase::FLIPPED_DIAGONALLY_FLAG    = 0x20000000;
const unsigned TilesetBase::ROTATED_HEXAGONAL_120_FLAG = 0x10000000;   

Tileset::Tileset(TextureManager &texMan_, uint32_t firstgid_) :
    m_texManager(texMan_),
    m_firstgid(firstgid_)
{
}

void Tileset::load(const std::string &spritesheet_)
{
    m_tiles.clear();

    m_tex = m_texManager.getTexture(m_texManager.getTexID(spritesheet_));

    const auto tilesetTilesWidth = m_tex->m_size.x / gamedata::tiles::tileSize.x;
    const auto tilesetTilesHeight = m_tex->m_size.y / gamedata::tiles::tileSize.y;

    for (m_size.y = 0; m_size.y < tilesetTilesHeight; ++m_size.y)
    {
        for (m_size.x = 0; m_size.x < tilesetTilesWidth; ++m_size.x)
        {
            m_tiles.push_back({m_tex->m_id, Vector2{m_size.x * gamedata::tiles::tileSize.x, m_size.y * gamedata::tiles::tileSize.y}});
        }
    }
}

TileView *Tileset::getView(uint32_t id_)
{
    return &m_tiles[id_ - m_firstgid];
}

TilesetBase::TilesetBase(TextureManager &texMan_) :
    m_texManager(texMan_)
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
    std::cout << "Loading tileset from sprite \"" << spritesheet_ << "\" with first gid " << firstgid_ << std::endl;
    m_tilesets.emplace_back(m_texManager, firstgid_);
    m_tilesets.back().load(spritesheet_);

    m_tilesetMapping.addPair(firstgid_, m_tilesets.size() - 1);
}

Tile TilesetBase::getTile(uint32_t gid_)
{
    auto flags = flagsToFlip(gid_);
    gid_ &= ~(FLIPPED_HORIZONTALLY_FLAG |
        FLIPPED_VERTICALLY_FLAG |
        FLIPPED_DIAGONALLY_FLAG |
        ROTATED_HEXAGONAL_120_FLAG);

    const auto tilesetId = m_tilesetMapping[gid_];
    return {m_tilesets[tilesetId].getView(gid_), flags};
}

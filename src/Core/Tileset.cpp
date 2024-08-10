#include "Tileset.h"

Tileset::Tileset(Application *application_) :
    m_texManager(*application_->getTextureManager())
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

Tile Tileset::getTile(const Vector2<float> pos_, unsigned gid_)
{
    auto flags = flagsToFlip(gid_);
    gid_ &= ~(FLIPPED_HORIZONTALLY_FLAG |
        FLIPPED_VERTICALLY_FLAG |
        FLIPPED_DIAGONALLY_FLAG |
        ROTATED_HEXAGONAL_120_FLAG);

    return {m_tiles[gid_ - 1], pos_, flags};
}

SDL_RendererFlip Tileset::flagsToFlip(unsigned guid_) const
{
    SDL_RendererFlip res = SDL_FLIP_NONE;

    if (guid_ & FLIPPED_HORIZONTALLY_FLAG)
        res = SDL_RendererFlip(res | SDL_FLIP_HORIZONTAL);

    if (guid_ & FLIPPED_VERTICALLY_FLAG)
        res = SDL_RendererFlip(res | SDL_FLIP_VERTICAL);

    return res;
}

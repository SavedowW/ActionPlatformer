#include "DecorLayers.h"

DecorLayers::DecorLayers(Application *application_) :
    m_renderer(*application_->getRenderer())
{
}

void DecorLayers::insert(size_t layerId_, Tile &&tile_)
{
    if (layerId_ >= m_layers.size())
        throw std::string("Trying to insert into a layer that does not exist");

    m_layers[layerId_].m_tiles.push_back(std::move(tile_));
}

void DecorLayers::draw(size_t layerId_, const Camera &cam_)
{
    if (layerId_ >= m_layers.size())
        throw std::string("Trying to render layer that does not exist");

    for (auto &el : m_layers[layerId_].m_tiles)
        m_renderer.renderTexture(el.m_tile.m_tex, el.m_pos.x, el.m_pos.y, gamedata::global::tileSize.x, gamedata::global::tileSize.y, el.m_tile.m_src, cam_, el.m_flip);
}

void DecorLayers::draw(const Camera &cam_)
{
    for (auto &layer : m_layers)
    {
        auto old = m_renderer.setRenderTarget(layer.m_tex.getSprite());
        m_renderer.fillRenderer({255, 255, 255, 0});
        for (auto &el : layer.m_tiles)
        {
            m_renderer.renderTexture(el.m_tile.m_tex, el.m_pos.x, el.m_pos.y, gamedata::global::tileSize.x, gamedata::global::tileSize.y, el.m_tile.m_src, cam_, el.m_flip);
        }
        m_renderer.setRenderTarget(old);
        m_renderer.renderTexture(layer.m_tex.getSprite(), 0, 0, layer.m_tex.m_w, layer.m_tex.m_h);
    }
}

size_t DecorLayers::createLayer(const SDL_Color &colMul_)
{
    auto tex = m_renderer.createTexture(gamedata::global::maxCameraSize);
    SDL_SetTextureColorMod(tex, colMul_.r, colMul_.g, colMul_.b);
    SDL_SetTextureAlphaMod(tex, colMul_.a);
    m_layers.push_back({tex});
    return m_layers.size() - 1;
}

DecorLayers::DecorLayers(DecorLayers &&rhs_) :
    m_renderer(rhs_.m_renderer)
{
    m_layers = std::move(rhs_.m_layers);
}

DecorLayers &DecorLayers::operator=(DecorLayers &&rhs_)
{
    m_layers = std::move(rhs_.m_layers);

    return *this;
}

TextureLayer::TextureLayer(SDL_Texture *tex_) :
    m_tex(tex_)
{
}

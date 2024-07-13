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

        Vector2<int> camTL = Vector2<int>(cam_.getPos().mulComponents(layer.m_parallaxFactor)) - Vector2<int>(gamedata::global::maxCameraSize) / 2;
        //camTL = (camTL - cam_.getSize() / 2).mulComponents(layer.m_parallaxFactor);

        SDL_Rect dst;
        dst.w = gamedata::global::tileSize.x;
        dst.h = gamedata::global::tileSize.y;

        for (auto &el : layer.m_tiles)
        {
            dst.x = el.m_pos.x - camTL.x;
            dst.y = el.m_pos.y - camTL.y;
            m_renderer.renderTexture(el.m_tile.m_tex, &el.m_tile.m_src, &dst, 0, nullptr, el.m_flip);
        }
        m_renderer.setRenderTarget(old);
        m_renderer.renderTexture(layer.m_tex.getSprite(), 0, 0, layer.m_tex.m_w, layer.m_tex.m_h);
    }
}

size_t DecorLayers::createLayer(const SDL_Color &colMul_, const Vector2<float> &parallaxFactor_)
{
    auto tex = m_renderer.createTexture(gamedata::global::maxCameraSize);
    SDL_SetTextureColorMod(tex, colMul_.r, colMul_.g, colMul_.b);
    SDL_SetTextureAlphaMod(tex, colMul_.a);
    m_layers.push_back({tex, parallaxFactor_});
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

TextureLayer::TextureLayer(SDL_Texture *tex_, const Vector2<float> &parallaxFactor_) :
    m_tex(tex_),
    m_parallaxFactor(parallaxFactor_)
{
}

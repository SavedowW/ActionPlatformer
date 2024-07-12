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
        for (auto &el : layer.m_tiles)
            m_renderer.renderTexture(el.m_tile.m_tex, el.m_pos.x, el.m_pos.y, gamedata::global::tileSize.x, gamedata::global::tileSize.y, el.m_tile.m_src, cam_, el.m_flip);
}

size_t DecorLayers::createLayer()
{
    m_layers.push_back({});
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

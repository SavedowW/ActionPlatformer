#ifndef DECOR_LAYERS_H_
#define DECOR_LAYERS_H_
#include "StaticDecor.h"
#include <vector>
#include <array>
#include <memory>
#include "Tileset.h"

struct TextureLayer
{
    std::vector<Tile> m_decors;
};

template<size_t LAYER_COUNT>
class DecorLayers
{
public:
    DecorLayers(Application *application_) :
        m_renderer(*application_->getRenderer())
    {
    }

    template<size_t LAYER_ID>
    void insert(Tile &&tile_)
    {
        static_assert(LAYER_ID < LAYER_COUNT, "Trying to insert into a layer that does not exist");

        m_layers[LAYER_ID].m_decors.push_back(std::move(tile_));
    }

    template<size_t LAYER_ID>
    void draw(const Camera &cam_)
    {
        static_assert(LAYER_ID < LAYER_COUNT, "Trying to render layer that does not exist");

        for (auto &el : m_layers[LAYER_ID].m_decors)
        {
            m_renderer.renderTexture(el.m_tile.m_tex, el.m_pos.x, el.m_pos.y, gamedata::global::tileSize.x, gamedata::global::tileSize.y, el.m_tile.m_src, cam_);
        }
    }

private:
    Renderer &m_renderer;
    std::array<TextureLayer, LAYER_COUNT> m_layers;
    
};

#endif
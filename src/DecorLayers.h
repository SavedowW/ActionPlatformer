#ifndef DECOR_LAYERS_H_
#define DECOR_LAYERS_H_
#include "StaticDecor.h"
#include <vector>
#include <memory>
#include "Tileset.h"

struct TextureLayer
{
    std::vector<Tile> m_tiles;
};

class DecorLayers
{
public:
    DecorLayers(Application *application_);

    void insert(size_t layerId_, Tile &&tile_);

    void draw(size_t layerId_, const Camera &cam_);
    void draw(const Camera &cam_);

    size_t createLayer();

    DecorLayers(DecorLayers &&rhs_);
    DecorLayers &operator=(DecorLayers &&rhs_);

private:
    Renderer &m_renderer;
    std::vector<TextureLayer> m_layers;
    
};

#endif
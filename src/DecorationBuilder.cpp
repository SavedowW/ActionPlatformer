#include "DecorationBuilder.h"
#include <fstream>
#include "TileMapHelper.hpp"

DecorationBuilder::DecorationBuilder(Application &app_) :
    m_app(app_),
    m_root(app_.getBasePath())
{
}

DecorLayers DecorationBuilder::buildDecorLayers(const std::string &mapDescr_, Tileset &usedTileset_)
{
    auto fullpath = m_root + "/" + mapDescr_;
    DecorLayers dmap(&m_app);

    std::ifstream mapjson(fullpath);
    if (!mapjson.is_open())
    {
        std::cout << "Failed to open map description at \"" << fullpath << "\"\n";
        return dmap;
    }

    nlohmann::json mapdata = nlohmann::json::parse(mapjson);
    for (const auto &layer : mapdata["layers"])
    {
        int height = layer["height"];
        int width = layer["width"];

        SDL_Color layerCfg = {255, 255, 255, 255};
        Vector2<float> parallaxFactor {1.0f, 1.0f};
        if (layer.contains("tintcolor"))
            layerCfg = utils::hexToCol<true>(layer["tintcolor"]);

        if (layer.contains("opacity"))
            layerCfg.a = (double)layer["opacity"] * 255;

        if (layer.contains("parallaxx"))
            parallaxFactor.x = layer["parallaxx"];

        if (layer.contains("parallaxy"))
            parallaxFactor.y = layer["parallaxy"];

        int pos = 0;
        auto layerid = dmap.createLayer(layerCfg, parallaxFactor);
        for (const auto &tile : layer["data"])
        {
            unsigned int gid = tile;
            if (gid != 0)
                dmap.insert(layerid, usedTileset_.getTile(getTilePos(Vector2{pos % width, pos / width}), gid));
            pos++;
        }
    }

    return dmap;
}

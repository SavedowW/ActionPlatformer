#include "LevelBuilder.h"
#include "TileMapHelper.hpp"
#include "CoreComponents.h"
#include <fstream>
#include <limits>

void addTrigger(entt::registry &reg_, const Trigger &trg_)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentTrigger>(newid, trg_);
}

LevelBuilder::LevelBuilder(Application &app_, entt::registry &reg_) :
    m_app(app_),
    m_root(app_.getBasePath()),
    m_reg(reg_)
{
}

DecorLayers LevelBuilder::buildLevel(const std::string &mapDescr_, Tileset &usedTileset_, entt::entity playerId_)
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
        std::cout << "Loading " << layer["name"] << " of type " << layer["type"] << std::endl;
        if (layer["type"] == "tilelayer")
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
        else if (layer["type"] == "objectgroup")
        {
            if (layer["name"] == "Meta")
            {
                for (const auto &obj : layer["objects"])
                {
                    if (obj["type"] == "SpawnPoint")
                    m_reg.emplace_or_replace<ComponentSpawnLocation>(playerId_, Vector2{
                        static_cast<float>(obj["x"]),
                        static_cast<float>(obj["y"])
                    });
                }
            }
            else if (layer["name"] == "Collision")
            {
                for (const auto &cld : layer["objects"])
                {
                    SlopeCollider scld;
                    int obstacleId = 0;

                    Vector2<float> tl{
                            static_cast<float>(cld["x"]),
                            static_cast<float>(cld["y"])
                        };

                    if (cld.contains("polygon"))
                    {
                        float minx = std::numeric_limits<float>::max();
                        float maxx = std::numeric_limits<float>::min();

                        float miny_at_minx = std::numeric_limits<float>::max();
                        float miny_at_maxx = std::numeric_limits<float>::max();

                        float maxy_at_minx = std::numeric_limits<float>::min();
                        float maxy_at_maxx = std::numeric_limits<float>::min();

                        for (const auto &vertex : cld["polygon"])
                        {
                            Vector2<float> vvx = tl + Vector2{
                                static_cast<float>(vertex["x"]),
                                static_cast<float>(vertex["y"])
                            };

                            minx = std::min(minx, vvx.x);
                            maxx = std::max(maxx, vvx.x);

                            if (vvx.x == minx)
                            {
                                miny_at_minx = std::min(miny_at_minx, vvx.y);
                                maxy_at_minx = std::max(maxy_at_minx, vvx.y);
                            }
                            else if (vvx.x == maxx)
                            {
                                miny_at_maxx = std::min(miny_at_maxx, vvx.y);
                                maxy_at_maxx = std::max(maxy_at_maxx, vvx.y);
                            }
                            else
                            {
                                throw std::exception("Failed to read polygon vertex for collider: x coord is not min or max");
                            }
                        }

                        Vector2<float> points[4] = {
                            {minx, miny_at_minx},
                            {maxx, miny_at_maxx},
                            {maxx, maxy_at_maxx},
                            {minx, maxy_at_minx},
                        };

                        scld.set(points);
                    }
                    else
                    {
                        Vector2<float> size{
                            static_cast<float>(cld["width"]),
                            static_cast<float>(cld["height"])
                        };

                        scld.set(tl, size, 0);
                    }

                    if (cld.contains("properties"))
                    {
                        for (const auto &prop : cld["properties"])
                        {
                            if (prop["name"] == "ObstacleGroup")
                                obstacleId = static_cast<int>(prop["value"]);
                        }
                    }
   
                    addCollider(scld, obstacleId);
                }
            }
            else if (layer["name"] == "Navigation")
            {
                std::cout << "WARNING: Navigation parsing is not implemented yet" << std::endl;
            }
            else if (layer["name"] == "Focus areas")
            {
                std::cout << "WARNING: Focus areas parsing is not implemented yet" << std::endl;
            }
        }
    }

    return dmap;
}

void LevelBuilder::addCollider(const SlopeCollider &worldCld_, int obstacleId_)
{
    auto newid = m_reg.create();
    auto &tr = m_reg.emplace<ComponentTransform>(newid, worldCld_.m_tlPos, ORIENTATION::RIGHT);
    m_reg.emplace<ComponentStaticCollider>(newid, ComponentStaticCollider(tr.m_pos, SlopeCollider({0.0f, 0.0f}, worldCld_.m_size, worldCld_.m_topAngleCoef), obstacleId_));
}

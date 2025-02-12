#include "LevelBuilder.h"
#include "NavGraph.h"
#include "TileMapHelper.hpp"
#include "CoreComponents.h"
#include "CameraFocusArea.h"
#include "StateMachine.h"
#include "ColliderRouting.h"
#include <fstream>
#include <limits>
#include <sstream>

void addTrigger(entt::registry &reg_, const Trigger &trg_)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentTrigger>(newid, trg_);
}

LevelBuilder::LevelBuilder(Application &app_, entt::registry &reg_) :
    m_app(app_),
    m_root(app_.getBasePath()),
    m_reg(reg_),
    m_tilebase(app_)
{
}

void LevelBuilder::buildLevel(const std::string &mapDescr_,entt::entity playerId_, NavGraph &graph_, ColliderRoutesCollection &rtCollection_)
{
    auto fullpath = m_root + "/" + mapDescr_;

    std::ifstream mapjson(fullpath);
    if (!mapjson.is_open())
    {
        std::cout << "Failed to open map description at \"" << fullpath << "\"\n";
        return;
    }

    nlohmann::json mapdata = nlohmann::json::parse(mapjson);

    // Parsing tilesets
    for (const auto &jsonTileset : mapdata["tilesets"])
    {
        auto firstgid = static_cast<int>(jsonTileset["firstgid"]);
        auto filename = static_cast<std::string>(jsonTileset["source"]);
        filename = filename.substr(0, filename.size() - 4);
        filename = "Tiles/" + filename;
        m_tilebase.addTileset(filename, firstgid);
    }

    int layerId = mapdata.size();

    for (const auto &layer : mapdata["layers"])
    {
        layerId--;
        
        std::cout << "Loading " << layer["name"] << " of type " << layer["type"] << std::endl;
        if (layer["type"] == "tilelayer")
        {
            Vector2<int> pos;
            pos.y = (layer.contains("offsety") ? static_cast<int>(layer["offsety"]) : 0);
            pos.x = (layer.contains("offsetx") ? static_cast<int>(layer["offsetx"]) : 0);

            Vector2<int> size;
            size.y = layer["height"];
            size.x = layer["width"];

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

            auto tex = m_app.getRenderer()->createTexture(gamedata::global::maxCameraSize);
            SDL_SetTextureColorMod(tex, layerCfg.r, layerCfg.g, layerCfg.b);
            SDL_SetTextureAlphaMod(tex, layerCfg.a);

            auto entity = m_reg.create();
            auto &tilelayer = m_reg.emplace<TilemapLayer>(entity, tex, size, parallaxFactor);
            m_reg.emplace<ComponentTransform>(entity, pos, ORIENTATION::RIGHT);
            m_reg.emplace<RenderLayer>(entity, layerId);

            int tileLinearPos = 0;
            for (const uint32_t tile : layer["data"])
            {
                if (tile)
                {
                    uint32_t gid = tile;
                    Vector2<int> tilePos;
                    tilePos.x = tileLinearPos % size.x;
                    tilePos.y = tileLinearPos / size.x;

                    tilelayer.m_tiles[tilePos.y][tilePos.x] = m_tilebase.getTile(gid);
                }

                tileLinearPos++;
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
                std::map<int, NodeID> nodes;
                std::map<std::pair<int, int>, std::pair<Traverse::TraitT, Traverse::TraitT>> connections;

                for (const auto &point : layer["objects"])
                {
                    Vector2<float> pos {
                                static_cast<float>(point["x"]),
                                static_cast<float>(point["y"])
                            };

                    nodes[static_cast<int>(point["id"])] = graph_.makeNode(pos);
                }

                for (const auto &point : layer["objects"])
                {
                    if (point.contains("properties"))
                    {
                        auto src = static_cast<int>(point["id"]);
                        for (const auto &prop : point["properties"])
                        {
                            auto traits = lineToTraverse(prop["name"]);
                            auto dst = static_cast<int>(prop["value"]);
                            
                            bool swapped = false;
                            if (src < dst)
                            {
                                connections[{src, dst}].first = traits;
                            }
                            else
                            {
                                connections[{dst, src}].second = traits;
                            }

                        }
                    }
                }

                for (auto &con : connections)
                {
                    graph_.makeConnection(nodes[con.first.first], nodes[con.first.second], con.second.first, con.second.second);
                }
            }
            else if (layer["name"] == "Focus areas")
            {
                std::map<int, Collider> triggerAreas;
                for (const auto &area : layer["objects"])
                {
                    int id = area["id"];
                    std::string type = area["type"];
                    if (type == "FocusTrigger")
                    {
                        Vector2<float> tl{area["x"], area["y"]};
                        Vector2<float> size{area["width"], area["height"]};
                        triggerAreas.emplace(id, Collider(tl + size / 2.0f, size / 2.0f));
                    }
                    else if (type == "FocusBorder")
                    {
                        Vector2<float> tl = {area["x"], area["y"]};
                        Vector2<float> size = {area["width"], area["height"]};

                        auto newfocus = m_reg.create();
                        m_reg.emplace<CameraFocusArea>(newfocus, tl + size / 2, size, *m_app.getRenderer());

                        if (area.contains("properties"))
                        {
                            for (auto &prop : area["properties"])
                            {
                                if (prop["name"] == "FocusTrigger" && prop["type"] == "object")
                                    m_reg.get<CameraFocusArea>(newfocus).overrideFocusArea(triggerAreas.at(prop["value"]));
                                else
                                    std::cout << "Unknown property \"" << prop["name"] << "\" of type \"" << prop["type"] << "\"" << std::endl;
                            }
                        }
                    }
                    else
                    {
                        std::cout << "Unknown area type at Focus areas: \"" << type << "\" (" << id << ")" << std::endl;
                    }
                }
            }
            else if (layer["name"] == "ColliderRouting")
            {
                struct PointDescr
                {
                    std::string initialLink;
                    Vector2<float> pos;
                    std::map<std::string, int> links;
                    std::map<int, std::string> rules;
                };

                std::map<int, PointDescr> points;

                for (const auto &obj : layer["objects"])
                {
                    auto &newpoint = points[static_cast<int>(obj["id"])];
                    newpoint.pos.x = static_cast<int>(obj["x"]);
                    newpoint.pos.y = static_cast<int>(obj["y"]);

                    std::cout << "New point at " << static_cast<int>(obj["id"]) << std::endl;
                    std::cout << newpoint.pos << std::endl;

                    if (obj.contains("properties"))
                    {
                        for (const auto &prop : obj["properties"])
                        {
                            if (prop["name"] == "InitialRoute")
                            {
                                std::cout << "Initial route: " << utils::wrap(prop["value"]) << std::endl;
                                newpoint.initialLink = prop["value"];
                            }
                            else if (utils::startsWith(prop["name"], "LINK"))
                            {
                                std::cout << "New link " << utils::wrap(prop["name"]) << " to " << prop["value"] << std::endl;
                                newpoint.links[prop["name"]] = prop["value"];
                            }
                            else if (utils::startsWith(prop["name"], "RouteRule"))
                            {
                                std::string ruledescr = prop["value"];
                                std::stringstream ss(ruledescr);
                                int from;
                                std::string to;
                                ss >> from;
                                ss >> to;
                                newpoint.rules[from] = to;
                                std::cout << "New rule from " << from << " to " << utils::wrap(to) << std::endl;
                            }
                        }
                    }
                }

                // Data ready, put it into the collection
                for (const auto &point : points)
                {
                    if (!point.second.initialLink.empty())
                    {
                        auto &newroute = rtCollection_.routes[point.first];
                        newroute.m_origin.m_id = point.first;
                        newroute.m_origin.m_pos = point.second.pos;

                        auto currentLink = point.second.initialLink;
                        int currentPoint = point.first;
                        while (newroute.m_links.empty() || newroute.m_origin.m_id != currentPoint && !currentLink.empty())
                        {
                            auto &newlnk = newroute.m_links.emplace_back();
                            newlnk.m_target.m_id = points[currentPoint].links[currentLink];
                            newlnk.m_target.m_pos = points[newlnk.m_target.m_id].pos;

                            auto oldPoint = currentPoint;
                            currentPoint = newlnk.m_target.m_id;

                            if (points[currentPoint].rules.contains(oldPoint))
                                currentLink = points[currentPoint].rules[oldPoint];
                            else
                                currentLink = "";
                        }
                    }
                }

            }
        }
    }
}

void LevelBuilder::addCollider(const SlopeCollider &worldCld_, int obstacleId_)
{
    auto newid = m_reg.create();
    auto &tr = m_reg.emplace<ComponentTransform>(newid, worldCld_.m_tlPos, ORIENTATION::RIGHT);
    m_reg.emplace<ComponentStaticCollider>(newid, ComponentStaticCollider(tr.m_pos, SlopeCollider({0.0f, 0.0f}, worldCld_.m_size, worldCld_.m_topAngleCoef), obstacleId_));
}

Traverse::TraitT LevelBuilder::lineToTraverse(const std::string &line_) const
{
    std::vector<TraverseTraits> traits;
    bool requireFallthrough = false;
    std::istringstream iss(line_);
    std::string s;
    getline( iss, s, ' ' );
    while (getline( iss, s, ' ' ) )
    {
        if (s == "W")
            traits.push_back(TraverseTraits::WALK);
        else if (s == "J")
            traits.push_back(TraverseTraits::JUMP);
        else if (s == "F")
            traits.push_back(TraverseTraits::FALL);
        else if (s == "D")
            requireFallthrough = true;
        else
            std::cout << "Warning: unknown trait identifier \"" << s << "\" at \"" << line_ << "\"" << std::endl;
    }

    auto sig = Traverse::makeSignature(requireFallthrough);
    for (auto &el : traits)
        sig = Traverse::extendSignature(sig, el);

    return sig;
}

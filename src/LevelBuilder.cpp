#include "LevelBuilder.h"
#include "EnvComponents.h"
#include "JsonUtils.hpp"
#include "NavGraph.h"
#include "CoreComponents.h"
#include "CameraFocusArea.h"
#include "StateMachine.h"
#include "ResetHandlers.h"
#include "FilesystemUtils.h"
#include <fstream>
#include <limits>
#include <sstream>

template <>
void LevelBuilder::makeObject<GrassTopComp>(const Vector2<int> &pos_, bool visible_, int layer_)
{
    auto &animManager = m_app.getAnimationManager();

    auto objEnt = m_reg.create();
    auto &trans = m_reg.emplace<ComponentTransform>(objEnt, pos_, ORIENTATION::RIGHT);

    auto &animrnd = m_reg.emplace<ComponentAnimationRenderable>(objEnt);
    auto &renLayer = m_reg.emplace<RenderLayer>(objEnt, layer_);
    renLayer.m_visible = visible_;

    animrnd.loadAnimation(animManager, animManager.getAnimID("Environment/grass_single_top"));
    animrnd.loadAnimation(animManager, animManager.getAnimID("Environment/grass_single_top_flickL"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(animManager, animManager.getAnimID("Environment/grass_single_top_flickR"), LOOPMETHOD::NOLOOP);

    auto animSize = animrnd.m_animations.at(animManager.getAnimID("Environment/grass_single_top")).getSize();
    auto animOrigin = animrnd.m_animations.at(animManager.getAnimID("Environment/grass_single_top")).getOrigin();

    animrnd.m_currentAnimation = &animrnd.m_animations.at(animManager.getAnimID("Environment/grass_single_top"));
    animrnd.m_currentAnimation->reset();

    //trans.m_pos.x += (animSize.x);
    //trans.m_pos.y -= (animSize.y);
    trans.m_pos.x += (animOrigin.x - 1);
    trans.m_pos.y -= (animSize.y + 1 - animOrigin.y);

    m_reg.emplace<GrassTopComp>(objEnt);
    GrassTopComp::m_idleAnimId = animManager.getAnimID("Environment/grass_single_top");
    GrassTopComp::m_flickRightAnimId = animManager.getAnimID("Environment/grass_single_top_flickR");
    GrassTopComp::m_flickLeftAnimId = animManager.getAnimID("Environment/grass_single_top_flickL");
}

void addTrigger(entt::registry &reg_, const Trigger &trg_)
{
    auto newid = reg_.create();
    reg_.emplace<ComponentTrigger>(newid, trg_);
}

LevelBuilder::LevelBuilder(Application &app_, entt::registry &reg_) :
    m_app(app_),
    m_reg(reg_),
    m_tilebase(app_.getTextureManager())
{
#define ADD_NAME_FACTORY_PAIR(classname) m_factories.emplace(#classname , &LevelBuilder::makeObject<classname>)

    ADD_NAME_FACTORY_PAIR(GrassTopComp);
}

void LevelBuilder::buildLevel(const std::string &mapDescr_, entt::entity playerId_, NavGraph &graph_, ColliderRoutesCollection &rtCollection_, EnvironmentSystem &env_)
{
    const auto fullpath = Filesystem::getRootDirectory() + mapDescr_;

    std::ifstream mapjson(fullpath);
    if (!mapjson.is_open())
    {
        std::cout << "Failed to open map description at \"" << fullpath << "\"\n";
        return;
    }

    nlohmann::json mapdata = nlohmann::json::parse(mapjson);

    // Preparing layer queue
    std::vector<LayerDescr> layers;
    for (const auto &layer : mapdata["layers"])
    {
        layers.emplace_back(layer);
    }

    std::sort(layers.begin(), layers.end(), [](const LayerDescr &lhs_, const LayerDescr &rhs_){
        return lhs_.m_priority < rhs_.m_priority;
    });

    // Parsing tilesets
    for (const auto &jsonTileset : mapdata["tilesets"])
    {
        std::filesystem::path jsonpath(static_cast<std::string>(jsonTileset["source"]));
        if (jsonpath.filename().string().substr(0, 4) == "util")
        {
            loadUtilTileset(std::filesystem::path(fullpath).parent_path() / jsonpath, jsonTileset["firstgid"]);
        }
        else
            loadTileset(std::filesystem::path(fullpath).parent_path() / jsonpath, jsonTileset["firstgid"]);
    }

    m_colliderIds.clear();
    m_autoLayer = mapdata.size();

    // Actually parsing layers
    for (const auto &layer : layers)
    {
        m_autoLayer--;
        
        std::cout << "Loading " << (*layer.m_layer)["name"] << " of type " << (*layer.m_layer)["type"] << std::endl;
        if ((*layer.m_layer)["type"] == "tilelayer")
        {
            loadTileLayer(*layer.m_layer);
        }
        else if ((*layer.m_layer)["type"] == "objectgroup")
        {
            if ((*layer.m_layer)["name"] == "Meta")
            {
                loadMetaLayer(*layer.m_layer, playerId_);
            }
            else if ((*layer.m_layer)["name"] == "Collision")
            {
                loadCollisionLayer(*layer.m_layer, rtCollection_);
            }
            else if ((*layer.m_layer)["name"] == "Navigation")
            {
                loadNavigationLayer(*layer.m_layer, graph_);
            }
            else if ((*layer.m_layer)["name"] == "Focus areas")
            {
                loadFocusLayer(*layer.m_layer);
            }
            else if ((*layer.m_layer)["name"] == "ColliderRouting")
            {
                loadColliderRoutingLayer(*layer.m_layer, rtCollection_);
            }
            else
            {
                loadObjectsLayer(*layer.m_layer, env_);
            }
        }
    }
}

entt::entity LevelBuilder::addCollider(const SlopeCollider &worldCld_, int obstacleId_, ColliderPointRouting *route_)
{
    auto newid = m_reg.create();
    auto &tr = m_reg.emplace<ComponentTransform>(newid, worldCld_.m_tlPos, ORIENTATION::RIGHT);
    m_reg.emplace<ComponentStaticCollider>(newid, ComponentStaticCollider(tr.m_pos, SlopeCollider({0, 0}, worldCld_.m_size, worldCld_.m_topAngleCoef), obstacleId_));
    if (route_)
    {
        m_reg.emplace<MoveCollider2Points>(newid, worldCld_.m_tlPos + worldCld_.m_size / 2.0f - route_->m_origin.m_pos);
        m_reg.emplace<ColliderRoutingIterator>(newid, *route_);
        m_reg.emplace<ComponentResetStatic<MoveCollider2Points>>(newid);
        m_reg.emplace<ComponentResetStatic<ColliderRoutingIterator>>(newid);
    }

    return newid;
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

void LevelBuilder::loadTileset(const std::filesystem::path &jsonLoc_, uint32_t firstgid_)
{
    std::cout << "Loading normal tileset from \"" << jsonLoc_ << "\", first gid: " << firstgid_ << std::endl;

    std::ifstream tilesetjson(jsonLoc_);
    if (!tilesetjson.is_open())
    {
        std::cout << "Failed to open tileset description at \"" << jsonLoc_ << "\"\n";
        return;
    }

    nlohmann::json tilesetdata = nlohmann::json::parse(tilesetjson);

    std::filesystem::path imagePath(static_cast<std::string>(tilesetdata["image"]));
    if (imagePath.is_relative())
    {
        imagePath = jsonLoc_.parent_path() / imagePath;
    }

    imagePath = std::filesystem::weakly_canonical(imagePath);
    imagePath = std::filesystem::relative(imagePath, Filesystem::getRootDirectory() + "Resources/");
    std::cout << imagePath.string() << std::endl;

    auto type = imagePath.begin()->string();

    if (type == "Sprites")
    {
        std::string internalPath;
        bool first = true;
        for (const auto &el : imagePath)
        {
            if (!first)
            {
                if (!internalPath.empty())
                    internalPath += "/";

                internalPath += el.string();
            }

            first = false;
        }

        internalPath = Filesystem::removeExtention(internalPath);

        m_tilebase.addTileset(internalPath, firstgid_);
    }
    else if (type == "Animations")
    {
        throw std::logic_error("Animated tilesets are not implemented yet");
    }
    else
    {
        throw std::runtime_error("Tileset image is in neither animations nor sprites directory");
    }
}

void LevelBuilder::loadUtilTileset(const std::filesystem::path &jsonLoc_, uint32_t firstgid_)
{
    std::cout << "Loading utility tileset from \"" << jsonLoc_ << "\", first gid: " << firstgid_ << std::endl;

    std::ifstream tilesetjson(jsonLoc_);
    if (!tilesetjson.is_open())
    {
        std::cout << "Failed to open utility tileset description at \"" << jsonLoc_ << "\"\n";
        return;
    }

    nlohmann::json tilesetdata = nlohmann::json::parse(tilesetjson);

    for (const auto &tile : tilesetdata["tiles"])
    {
        m_utilTilesetFactories[firstgid_ + tile["id"]] = m_factories[tile["type"]];
    }
}

void LevelBuilder::loadTileLayer(const nlohmann::json &json_)
{
    auto depth = m_autoLayer;

    Vector2<int> pos;
    pos.y = utils::tryClaim(json_, "offsety", 0);
    pos.x = utils::tryClaim(json_, "offsetx", 0);

    Vector2<int> size;
    size.y = json_["height"];
    size.x = json_["width"];

    Vector2<float> parallaxFactor;

    parallaxFactor.x = utils::tryClaim(json_, "parallaxx", 1.0f);
    parallaxFactor.y = utils::tryClaim(json_, "parallaxy", 1.0f);

    entt::entity entity = entt::null;
    bool existingEntity = false;

    if (json_.contains("properties"))
    {
        for (const auto &prop : json_["properties"])
        {
            if (prop["name"] == "collider")
            {
                entity = m_colliderIds[static_cast<int>(prop["value"])];
                existingEntity = true;
            }
            else if (prop["name"] == "layer")
                depth = static_cast<int>(prop["value"]);
        }
    }

    if (entity == entt::null)
        entity = m_reg.create();

    auto &tilelayer = m_reg.emplace<TilemapLayer>(entity, size, parallaxFactor);
    if (!existingEntity)
        m_reg.emplace<ComponentTransform>(entity, pos, ORIENTATION::RIGHT);
    else
    {
        auto &trans = m_reg.get<ComponentTransform>(entity);
        tilelayer.m_posOffset = pos - trans.m_pos;
    }

    m_reg.emplace<RenderLayer>(entity, depth).m_visible = utils::tryClaim(json_, "visible", true);

    int tileLinearPos = 0;
    for (const uint32_t tile : json_["data"])
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

void LevelBuilder::loadMetaLayer(const nlohmann::json &json_, entt::entity playerId_)
{
    for (const auto &obj : json_["objects"])
    {
        if (obj["type"] == "SpawnPoint")
            m_reg.emplace_or_replace<ComponentSpawnLocation>(playerId_, Vector2{
                static_cast<float>(obj["x"]),
                static_cast<float>(obj["y"])
            });
    }
}

void LevelBuilder::loadCollisionLayer(const nlohmann::json &json_, ColliderRoutesCollection &rtCollection_)
{
    for (const auto &cld : json_["objects"])
    {
        int objectId = static_cast<int>(cld["id"]);
        SlopeCollider scld;
        int obstacleId = 0;
        ColliderPointRouting *route = nullptr;

        Vector2<int> tl{
                static_cast<int>(cld["x"]),
                static_cast<int>(cld["y"])
            };

        if (cld.contains("polygon"))
        {
            int minx = std::numeric_limits<int>::max();
            int maxx = std::numeric_limits<int>::min();

            int miny_at_minx = std::numeric_limits<int>::max();
            int miny_at_maxx = std::numeric_limits<int>::max();

            int maxy_at_minx = std::numeric_limits<int>::min();
            int maxy_at_maxx = std::numeric_limits<int>::min();

            for (const auto &vertex : cld["polygon"])
            {
                Vector2<int> vvx = tl + Vector2{
                    static_cast<int>(vertex["x"]),
                    static_cast<int>(vertex["y"])
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
                    throw std::logic_error("Failed to read polygon vertex for collider: x coord is not min or max");
                }
            }

            Vector2<int> points[4] = {
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
                else if (prop["name"] == "RoutingStart")
                    route = &rtCollection_.m_routes[static_cast<int>(prop["value"])];
            }
        }

        m_colliderIds[objectId] = addCollider(scld, obstacleId, route);
    }
}

void LevelBuilder::loadNavigationLayer(const nlohmann::json &json_, NavGraph &graph_)
{
    std::map<int, NodeID> nodes;
    std::map<std::pair<int, int>, std::pair<Traverse::TraitT, Traverse::TraitT>> connections;

    for (const auto &point : json_["objects"])
    {
        Vector2<float> pos {
                    static_cast<float>(point["x"]),
                    static_cast<float>(point["y"])
                };

        nodes[static_cast<int>(point["id"])] = graph_.makeNode(pos);
    }

    for (const auto &point : json_["objects"])
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

void LevelBuilder::loadFocusLayer(const nlohmann::json &json_)
{
    std::map<int, Collider> triggerAreas;
    for (const auto &area : json_["objects"])
    {
        int id = area["id"];
        std::string type = area["type"];
        if (type == "FocusTrigger")
        {
            Vector2<int> tl{area["x"], area["y"]};
            Vector2<int> size{area["width"], area["height"]};
            triggerAreas.emplace(id, Collider(tl, size));
        }
        else if (type == "FocusBorder")
        {
            Vector2<int> tl = {area["x"], area["y"]};
            Vector2<int> size = {area["width"], area["height"]};

            auto newfocus = m_reg.create();
            m_reg.emplace<CameraFocusArea>(newfocus, tl, size, m_app.getRenderer());

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

void LevelBuilder::loadColliderRoutingLayer(const nlohmann::json &json_, ColliderRoutesCollection &rtCollection_)
{
    struct PointDescr
    {
        std::string initialLink;
        Vector2<float> pos;
        std::map<std::string, int> links;
        std::map<int, std::string> rules;
    };

    std::map<int, PointDescr> points;

    for (const auto &obj : json_["objects"])
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
            auto &newroute = rtCollection_.m_routes[point.first];
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

void LevelBuilder::loadObjectsLayer(const nlohmann::json &json_, EnvironmentSystem &env_)
{
    auto depth = m_autoLayer;

    if (json_.contains("properties"))
    {
        for (const auto &prop : json_["properties"])
        {
            if (prop["name"] == "layer")
                depth = static_cast<int>(prop["value"]);
        }
    }

    for (const auto &obj : json_["objects"])
    {
        int gid = obj["gid"];
        Vector2<int> pos = {static_cast<int>(obj["x"]), static_cast<int>(obj["y"])};
        bool visible = obj["visible"];
        (this->*m_utilTilesetFactories.at(gid))(pos, visible, depth);
    }
}

LevelBuilder::LayerDescr::LayerDescr(const nlohmann::json &layer_) :
    m_layer(&layer_)
{
    if (layer_["name"] == "ColliderRouting")
        m_priority = 0;
    else if (layer_["name"] == "Collision")
        m_priority = 1;
    else if (layer_["type"] == "tilelayer")
        m_priority = 4;
    else
        m_priority = 3;
}

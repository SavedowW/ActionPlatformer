#include "LevelBuilder.h"
#include "EnvComponents.h"
#include "SM/StateMachine.h"
#include "ResetHandlers.h"
#include "Core/Application.h"
#include "Core/JsonUtils.hpp"
#include "Core/NavGraph.h"
#include "Core/CoreComponents.h"
#include "Core/CameraFocusArea.h"
#include "Core/FilesystemUtils.h"
#include "Core/Logger.hpp"
#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>

template <>
void LevelBuilder::makeObject<GrassTopComp>(const Vector2<int> &pos_, bool visible_, int layer_)
{
    auto &animManager = Application::instance().m_animationManager;

    auto objEnt = m_reg.create();
    auto &trans = m_reg.emplace<ComponentTransform>(objEnt, pos_, ORIENTATION::RIGHT);

    auto &animrnd = m_reg.emplace<ComponentAnimationRenderable>(objEnt);
    m_reg.emplace<RenderLayer>(objEnt, layer_, visible_);

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

LevelBuilder::LevelBuilder(entt::registry &reg_) :
    m_reg(reg_)
{
#define ADD_NAME_FACTORY_PAIR(classname) m_factories.emplace(#classname , &LevelBuilder::makeObject<classname>)

    ADD_NAME_FACTORY_PAIR(GrassTopComp);
}

void LevelBuilder::buildLevel(const std::string &mapDescr_, NavGraph &graph_, ColliderRoutesCollection &rtCollection_)
{
    const auto fullpath = Filesystem::getRootDirectory() + mapDescr_;

    std::ifstream mapjson(fullpath);
    if (!mapjson.is_open())
        throw std::runtime_error(std::format("Failed to open map description at \"{}\"", fullpath));

    nlohmann::json mapdata = nlohmann::json::parse(mapjson);

    // Preparing layer queue
    std::vector<LayerDescr> layers;
    for (const auto &layer : mapdata.at("layers"))
    {
        layers.emplace_back(layer);
    }

    std::ranges::sort(layers, [](const LayerDescr &lhs_, const LayerDescr &rhs_){
        return lhs_.m_priority < rhs_.m_priority;
    });

    // Parsing tilesets
    for (const auto &jsonTileset : mapdata.at("tilesets"))
    {
        std::filesystem::path jsonpath(static_cast<std::string>(jsonTileset.at("source")));
        if (jsonpath.filename().string().starts_with("util"))
            loadUtilTileset(std::filesystem::path(fullpath).parent_path() / jsonpath, jsonTileset.at("firstgid"));
        else
            loadTileset(std::filesystem::path(fullpath).parent_path() / jsonpath, jsonTileset.at("firstgid"));
    }

    m_colliderIds.clear();
    m_autoLayer = static_cast<int>(mapdata.size());

    // Actually parsing layers
    for (const auto &layer : layers)
    {
        m_autoLayer--;

        const std::string name = layer.m_layer->at("name");
        const std::string type = layer.m_layer->at("type");
        
        LOG_TRACE("Loading {} of type {}", name, type);
        if (type == "tilelayer")
        {
            loadTileLayer(*layer.m_layer);
        }
        else if (type == "objectgroup")
        {
            if (name == "Meta")
            {
                loadMetaLayer(*layer.m_layer);
            }
            else if (name == "Collision")
            {
                loadCollisionLayer(*layer.m_layer, rtCollection_);
            }
            else if (name == "Navigation")
            {
                loadNavigationLayer(*layer.m_layer, graph_);
            }
            else if (name == "Focus areas")
            {
                loadFocusLayer(*layer.m_layer);
            }
            else if (name == "ColliderRouting")
            {
                loadColliderRoutingLayer(*layer.m_layer, rtCollection_);
            }
            else
            {
                loadObjectsLayer(*layer.m_layer);
            }
        }
    }
}

entt::entity LevelBuilder::addCollider(const SlopeCollider &worldCld_, int obstacleId_, const ColliderPointRouting &route_)
{
    const auto newid = m_reg.create();
    const auto &tr = m_reg.emplace<ComponentTransform>(newid, worldCld_.topLeft(), ORIENTATION::RIGHT);
    m_reg.emplace<ComponentStaticCollider>(newid, ComponentStaticCollider(tr.m_pos, worldCld_.movedBy(-worldCld_.topLeft()), obstacleId_));

    m_reg.emplace<MoveCollider2Points>(newid, route_.m_origin.m_pos - worldCld_.topLeft());
    m_reg.emplace<ColliderRoutingIterator>(newid, route_);
    m_reg.emplace<ComponentResetStatic<MoveCollider2Points>>(newid);
    m_reg.emplace<ComponentResetStatic<ColliderRoutingIterator>>(newid);

    return newid;
}

entt::entity LevelBuilder::addCollider(const SlopeCollider &worldCld_, int obstacleId_)
{
    const auto newid = m_reg.create();
    const auto &tr = m_reg.emplace<ComponentTransform>(newid, worldCld_.topLeft(), ORIENTATION::RIGHT);
    m_reg.emplace<ComponentStaticCollider>(newid, ComponentStaticCollider(tr.m_pos, worldCld_.movedBy(-worldCld_.topLeft()), obstacleId_));
    
    return newid;
}

Traverse::TraitT LevelBuilder::lineToTraverse(const std::string &line_)
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
            LOG_WARNING("Warning: unknown trait identifier \"{}\" at \"{}\"", s, line_);
    }

    auto sig = Traverse::makeSignature(requireFallthrough);
    for (auto &el : traits)
        sig = Traverse::extendSignature(sig, el);

    return sig;
}

void LevelBuilder::loadTileset(const std::filesystem::path &jsonLoc_, uint32_t firstgid_)
{
    LOG_INFO("Loading normal tileset from \"{}\", first gid: {}", jsonLoc_.string(), firstgid_);

    std::ifstream tilesetjson(jsonLoc_);
    if (!tilesetjson.is_open())
    {
        LOG_ERROR("Failed to open tileset description at \"{}\"", jsonLoc_.string());
        return;
    }

    nlohmann::json tilesetdata = nlohmann::json::parse(tilesetjson);

    std::filesystem::path imagePath(tilesetdata.at("image"));
    if (imagePath.is_relative())
    {
        imagePath = jsonLoc_.parent_path() / imagePath;
    }

    imagePath = std::filesystem::weakly_canonical(imagePath);
    imagePath = std::filesystem::relative(imagePath, Filesystem::getRootDirectory() + "Resources/");

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
        throw std::logic_error("Animated tilesets are not implemented yet");
    else
        throw std::runtime_error("Tileset image is in neither animations nor sprites directory");
}

void LevelBuilder::loadUtilTileset(const std::filesystem::path &jsonLoc_, uint32_t firstgid_)
{
    LOG_INFO("Loading utility tileset from \"{}\", first gid: {}", jsonLoc_.string(), firstgid_);

    std::ifstream tilesetjson(jsonLoc_);
    if (!tilesetjson.is_open())
    {
        LOG_ERROR("Failed to open utility tileset description at \"{}\"", jsonLoc_.string());
        return;
    }

    nlohmann::json tilesetdata = nlohmann::json::parse(tilesetjson);

    for (const auto &tile : tilesetdata.at("tiles"))
        m_utilTilesetFactories.emplace(firstgid_ + tile.at("id").get<uint32_t>(), m_factories.at(tile.at("type")));
}

void LevelBuilder::loadTileLayer(const nlohmann::json &json_)
{
    auto depth = m_autoLayer;

    const Vector2<int> pos {
        utils::tryClaim(json_, "offsetx", 0),
        utils::tryClaim(json_, "offsety", 0)
    };

    const Vector2<int> size {
        utils::tryClaim(json_, "width", 0),
        utils::tryClaim(json_, "height", 0)
    };

    const Vector2<float> parallaxFactor {
        utils::tryClaim(json_, "parallaxx", 1.0f),
        utils::tryClaim(json_, "parallaxy", 1.0f)
    };

    entt::entity entity = entt::null;
    bool existingEntity = false;

    if (json_.contains("properties"))
    {
        for (const auto &prop : json_["properties"])
        {
            const std::string name = prop.at("name");
            if (name == "collider")
            {
                const auto found = m_colliderIds.find(prop.at("value"));
                if (found != m_colliderIds.end())
                {
                    entity = found->second;
                    existingEntity = true;
                }
                else
                    LOG_ERROR("Failed to find collider {} tied to the tile layer {}", prop.at("value").get<uint32_t>(), json_.at("id").get<int>());
            }
            else if (name == "layer")
                depth = prop.at("value");
            else
                LOG_ERROR("Unexpected property \"{}\"", name);
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

    m_reg.emplace<RenderLayer>(entity, depth, utils::tryClaim(json_, "visible", true));

    int tileLinearPos = 0;
    for (const uint32_t tile : json_.at("data"))
    {
        if (tile)
        {
            const uint32_t gid = tile;
            const Vector2<int> tilePos {
                tileLinearPos % size.x,
                tileLinearPos / size.x
            };

            tilelayer.m_tiles.at(tilePos.y).at(tilePos.x) = m_tilebase.getTile(gid);
        }

        tileLinearPos++;
    }
}

void LevelBuilder::loadMetaLayer(const nlohmann::json &json_)
{
    for (const auto &obj : json_.at("objects"))
    {
        const std::string type = obj.at("type");
        if (obj.at("type") == "SpawnPoint")
        {
            auto metaEntity = m_reg.create();
            m_reg.emplace<ComponentSpawnLocation>(metaEntity, Vector2<int>{
                obj.at("x"),
                obj.at("y")
            });
        }
            LOG_ERROR("Unexpected type at a meta layer \"{}\"", type);
    }
}

void LevelBuilder::loadCollisionLayer(const nlohmann::json &json_, const ColliderRoutesCollection &rtCollection_)
{
    for (const auto &cld : json_.at("objects"))
    {
        const int objectId = cld.at("id");
        SlopeCollider scld;
        int obstacleId = 0;
        auto route = rtCollection_.end();

        const Vector2<int> tl{
            cld.at("x"),
            cld.at("y")
        };

        if (cld.contains("polygon"))
        {
            int minx = std::numeric_limits<int>::max();
            int maxx = std::numeric_limits<int>::min();

            int miny_at_minx = std::numeric_limits<int>::max();
            int miny_at_maxx = std::numeric_limits<int>::max();

            int maxy = std::numeric_limits<int>::min();

            for (const auto &vertex : cld.at("polygon"))
            {
                const Vector2<int> vvx = tl.add<int>(vertex.at("x"), vertex.at("y"));

                minx = std::min(minx, vvx.x);
                maxx = std::max(maxx, vvx.x);

                maxy = std::max(maxy, vvx.y);

                if (vvx.x == minx)
                    miny_at_minx = std::min(miny_at_minx, vvx.y);
                else if (vvx.x == maxx)
                    miny_at_maxx = std::min(miny_at_maxx, vvx.y);
                else
                    throw std::logic_error("Failed to read polygon vertex for collider: x coord is not min or max");
            }

            scld.set({minx, miny_at_minx}, {maxx - 1, miny_at_maxx}, maxy - 1);
        }
        else
        {
            Vector2<int> size{
                static_cast<int>(cld.at("width")),
                static_cast<int>(cld.at("height"))
            };

            scld.set(tl, tl.add(size.x - 1, 0), tl.y + size.y - 1);
        }

        if (cld.contains("properties"))
        {
            for (const auto &prop : cld["properties"])
            {
                const std::string name = prop.at("name");

                if (name == "ObstacleGroup")
                    obstacleId = prop.at("value");
                else if (name == "RoutingStart")
                    route = rtCollection_.find(prop.at("value"));
                else
                    LOG_ERROR("Unexpected property \"{}\"", name);
            }
        }

        m_colliderIds[objectId] = (route != rtCollection_.end() ? addCollider(scld, obstacleId, route->second) : addCollider(scld, obstacleId));
    }
}

void LevelBuilder::loadNavigationLayer(const nlohmann::json &json_, NavGraph &graph_)
{
    std::map<int, NodeID> nodes;
    std::map<std::pair<int, int>, std::pair<Traverse::TraitT, Traverse::TraitT>> connections;

    for (const auto &point : json_.at("objects"))
    {
        const Vector2<float> pos {
            static_cast<float>(point.at("x")),
            static_cast<float>(point.at("y"))
        };

        nodes[point.at("id")] = graph_.makeNode(pos);
    }

    for (const auto &point : json_.at("objects"))
    {
        const int src = point.at("id");
        for (const auto &prop : point.at("properties"))
        {
            try
            {
                const auto traits = lineToTraverse(prop.at("name"));
                const auto dst = static_cast<int>(prop.at("value"));
                
                if (src < dst)
                    connections[{src, dst}].first = traits;
                else
                    connections[{dst, src}].second = traits;
            }
            catch (const std::exception &ex_)
            {
                LOG_ERROR("Failed to establish a connection from point {}: ", src, ex_.what());
            }

        }
    }

    for (auto &con : connections)
        graph_.makeConnection(nodes.at(con.first.first), nodes.at(con.first.second), con.second.first, con.second.second);
}

void LevelBuilder::loadFocusLayer(const nlohmann::json &json_)
{
    std::map<int, Collider> triggerAreas;
    for (const auto &area : json_.at("objects"))
    {
        const int id = area.at("id");
        const std::string type = area.at("type");

        if (type == "FocusTrigger")
        {
            const Vector2<int> tl{area.at("x"), area.at("y")};
            const Vector2<int> size{area.at("width"), area.at("height")};
            triggerAreas.emplace(id, Collider(tl, size));
        }
        else if (type == "FocusBorder")
        {
            try
            {
                Vector2<int> tl = {area.at("x"), area.at("y")};
                Vector2<int> size = {area.at("width"), area.at("height")};

                auto newfocus = m_reg.create();
                m_reg.emplace<CameraFocusArea>(newfocus, tl, size);

                for (const auto &prop : area.at("properties"))
                {
                    const std::string name = prop.at("name");
                    const std::string type = prop.at("type");

                    if (name == "FocusTrigger" && type == "object")
                        m_reg.get<CameraFocusArea>(newfocus).overrideFocusArea(triggerAreas.at(prop.at("value")));
                    else
                        LOG_ERROR(R"(Unexpected property "{}" of type "{}")", name, type);
                }
            }
            catch (const std::exception &ex_)
            {
                LOG_ERROR("Error while creating FocusBorder - most likely, a connected trigger is not created yet: {}", ex_.what());
            }
        }
        else
            LOG_ERROR("Unexpected type \"{}\"", type);
    }
}

void LevelBuilder::loadColliderRoutingLayer(const nlohmann::json &json_, ColliderRoutesCollection &rtCollection_)
{
    struct PointDescr
    {
        std::string initialLink;
        Vector2<int> pos;
        std::map<std::string, int> links;
        std::map<int, std::string> rules;
    };

    std::map<int, PointDescr> points;

    for (const auto &obj : json_.at("objects"))
    {
        auto &newpoint = points[obj.at("id")];
        newpoint.pos.x = obj.at("x");
        newpoint.pos.y = obj.at("y");

        if (obj.contains("properties"))
        {
            for (const auto &prop : obj["properties"])
            {
                const std::string name = prop.at("name");
                if (name == "InitialRoute")
                    newpoint.initialLink = prop.at("value");
                else if (utils::startsWith(name, "LINK"))
                    newpoint.links[name] = prop.at("value");
                else if (utils::startsWith(name, "RouteRule"))
                {
                    std::stringstream ss(prop.at("value").get<std::string>());
                    int from = 0;
                    std::string to;
                    ss >> from;
                    ss >> to;
                    newpoint.rules[from] = to;
                }
                else
                    LOG_ERROR("Unexpected property \"{}\"", name);
            }
        }
    }

    // Data ready, put it into the collection
    for (const auto &point : points)
    {
        if (!point.second.initialLink.empty())
        {
            auto &newroute = rtCollection_[point.first];
            newroute.m_origin.m_id = point.first;
            newroute.m_origin.m_pos = point.second.pos;

            auto currentLink = point.second.initialLink;
            int currentPoint = point.first;
            while (newroute.m_links.empty() || newroute.m_origin.m_id != currentPoint && !currentLink.empty())
            {
                auto &newlnk = newroute.m_links.emplace_back();
                newlnk.m_target.m_id = points[currentPoint].links[currentLink];
                newlnk.m_target.m_pos = points[newlnk.m_target.m_id].pos;

                const auto oldPoint = currentPoint;
                currentPoint = newlnk.m_target.m_id;

                if (points[currentPoint].rules.contains(oldPoint))
                    currentLink = points[currentPoint].rules[oldPoint];
                else
                    currentLink = "";
            }
        }
    }
}

void LevelBuilder::loadObjectsLayer(const nlohmann::json &json_)
{
    auto depth = m_autoLayer;

    bool layerVisible = json_.at("visible");

    for (const auto &prop : json_.at("properties"))
    {
        const std::string name = prop.at("name");
        if (name == "layer")
            depth = prop.at("value");
        else
            LOG_ERROR("Unexpected property \"{}\"", name);
    }

    for (const auto &obj : json_.at("objects"))
    {
        const uint32_t gid = obj.at("gid");
        const Vector2<int> pos = {obj.at("x"), obj.at("y")};
        const bool visible = obj.at("visible");
        (this->*m_utilTilesetFactories.at(gid))(pos, visible && layerVisible, depth);
    }
}

LevelBuilder::LayerDescr::LayerDescr(const nlohmann::json &layer_) :
    m_layer(&layer_)
{
    if (layer_.at("name") == "ColliderRouting")
        m_priority = 0;
    else if (layer_.at("name") == "Collision")
        m_priority = 1;
    else if (layer_.at("type") == "tilelayer")
        m_priority = 4;
    else
        m_priority = 3;
}

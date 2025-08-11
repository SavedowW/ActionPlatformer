#ifndef LEVEL_BUILDER_H_
#define LEVEL_BUILDER_H_
#include "ColliderRouting.h"
#include "Application.h"
#include "Tileset.h"
#include "EnvironmentSystem.h"
#include "StateMachine.h"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

class NavGraph;

class LevelBuilder
{
public:
    LevelBuilder(Application &app_, entt::registry &reg_);
    void buildLevel(const std::string &mapDescr_, entt::entity playerId_, NavGraph &graph_, ColliderRoutesCollection &rtCollection_);

private:
    /*
        Used to sort layers and process them in correct order

        1 - Collider routing
        2 - Colliders
        3 - Object layers
        4 - Tilemap layers
    */
    struct LayerDescr
    {
        LayerDescr(const nlohmann::json &layer_);

        const nlohmann::json *m_layer;
        int m_priority;
    };

    entt::entity addCollider(const SlopeCollider &worldCld_, int obstacleId_, ColliderPointRouting *route_);
    Traverse::TraitT lineToTraverse(const std::string &line_) const;

    void loadTileset(const std::filesystem::path &jsonLoc_, uint32_t firstgid_);
    void loadUtilTileset(const std::filesystem::path &jsonLoc_, uint32_t firstgid_);

    void loadTileLayer(const nlohmann::json &json_);
    void loadMetaLayer(const nlohmann::json &json_, entt::entity playerId_);
    void loadEnvLayer(const nlohmann::json &json_, EnvironmentSystem &env_);
    void loadCollisionLayer(const nlohmann::json &json_, ColliderRoutesCollection &rtCollection_);
    void loadNavigationLayer(const nlohmann::json &json_, NavGraph &graph_);
    void loadFocusLayer(const nlohmann::json &json_);
    void loadColliderRoutingLayer(const nlohmann::json &json_, ColliderRoutesCollection &rtCollection_);
    void loadObjectsLayer(const nlohmann::json &json_);


    // Object factories
    template<typename T>
    void makeObject(const Vector2<int> &pos_, bool visible_, int layer_) = delete;

    using FactoryMethod = void (LevelBuilder::*)(const Vector2<int>&, bool, int);

    std::map<std::string, FactoryMethod> m_factories;

    Application &m_app;
    entt::registry &m_reg;

    TilesetBase m_tilebase;

    std::map<int, entt::entity> m_colliderIds;
    std::unordered_map<int, FactoryMethod> m_utilTilesetFactories;

    int m_autoLayer;
};

#endif

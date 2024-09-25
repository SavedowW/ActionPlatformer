#ifndef DECORATION_BUILDER_H_
#define DECORATION_BUILDER_H_
#include "Application.h"
#include "DecorLayers.h"
#include "StateMachine.h"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

class NavGraph;

class LevelBuilder
{
public:
    LevelBuilder(Application &app_, entt::registry &reg_);
    DecorLayers buildLevel(const std::string &mapDescr_, Tileset &usedTileset_, entt::entity playerId_, NavGraph &graph_);


private:
    void addCollider(const SlopeCollider &worldCld_, int obstacleId_);
    Traverse::TraitT lineToTraverse(const std::string &line_) const;

    Application &m_app;
    const std::string m_root;
    entt::registry &m_reg;
};

#endif

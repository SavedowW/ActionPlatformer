#ifndef LEVEL_BUILDER_H_
#define LEVEL_BUILDER_H_
#include "ColliderRouting.h"
#include "Application.h"
#include "Tileset.h"
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
    entt::entity addCollider(const SlopeCollider &worldCld_, int obstacleId_, ColliderPointRouting *route_);
    Traverse::TraitT lineToTraverse(const std::string &line_) const;

    Application &m_app;
    entt::registry &m_reg;

    TilesetBase m_tilebase;
};

#endif

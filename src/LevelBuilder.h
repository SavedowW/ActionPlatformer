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
    void addCollider(const SlopeCollider &worldCld_, int obstacleId_);
    Traverse::TraitT lineToTraverse(const std::string &line_) const;

    Application &m_app;
    const std::string m_root;
    entt::registry &m_reg;

    TilesetBase m_tilebase;
};

#endif

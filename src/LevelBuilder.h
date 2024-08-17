#ifndef DECORATION_BUILDER_H_
#define DECORATION_BUILDER_H_
#include "Application.h"
#include "DecorLayers.h"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

class BattleLevel;
struct ComponentStaticCollider;

class LevelBuilder
{
public:
    LevelBuilder(Application &app_, entt::registry &reg_);
    DecorLayers buildLevel(const std::string &mapDescr_, Tileset &usedTileset_, entt::entity playerId_);


private:
    void addCollider(ComponentStaticCollider &&cld_);

    Application &m_app;
    const std::string m_root;
    entt::registry &m_reg;
};

#endif

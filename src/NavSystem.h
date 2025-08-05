#ifndef NAV_SYSTEM_H_
#define NAV_SYSTEM_H_
#include "Application.h"
#include "NavGraph.h"
#include "Camera.h"
#include <entt/entt.hpp>

// A view to a selected connection for pathfinding algo
struct ConnectionDescr
{
    ConnectionDescr(const Connection *con_);

    const Connection * const m_con;
    const float m_ownCost;

    // Actual total cost until target
    float m_calculatedCost;
    
    std::vector<ConnectionDescr *> m_neighbourConnections;
    std::optional<ConnectionDescr *> m_nextConnection;
    int m_nextNode = -1;
    std::pair<NodeID, NodeID> getOrientedNodes() const;
};

class NavPath
{
public:
    NavPath(const NavGraph &graph_, entt::entity target_, entt::registry &reg_, Traverse::TraitT traits_);

    bool buildUntil(const Connection * const con_);

    /*
        Read current connection of the target
        If it's changed - clear current costs and next connections
    */
    void update();
    void dump() const;

    const NavGraph &m_graph;
    entt::entity m_target = entt::null;
    std::vector<ConnectionDescr> m_fullGraph;
    const Connection *m_currentTarget;
    const Traverse::TraitT m_traverseTraits;
    entt::registry &m_reg;

    std::vector<ConnectionDescr *> front;
};

/*
    System, responsible for path management
    Each path is essentially a resource, identified by it's traverse traits and target entity
        (TODO, currently only traits)
    It's users share the same path and reuse constructed parts
*/
struct NavSystem
{
    NavSystem(entt::registry &reg_, Application &app_, NavGraph &graph_);

    void update();
    void draw(Camera &cam_);

    // Get existing path instance or create new
    std::shared_ptr<NavPath> makePath(Traverse::TraitT traverseTraits_, entt::entity goal_);

    entt::registry &m_reg;
    Renderer &m_ren;
    TextManager &m_textman;
    NavGraph &m_graph;

    std::unordered_map<Traverse::TraitT, std::weak_ptr<NavPath>> m_paths;
};

#endif

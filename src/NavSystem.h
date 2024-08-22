#ifndef NAV_SYSTEM_H_
#define NAV_SYSTEM_H_
#include "Application.h"
#include "NavGraph.h"
#include "Camera.h"
#include <entt/entt.hpp>

struct ConnectionDescr
{
    const Connection *m_con;
    float m_ownCost;
    float m_calculatedCost;
    std::vector<ConnectionDescr *> m_neighbourConnections;
    std::optional<ConnectionDescr *> m_nextConnection;
    int m_nextNode = -1;
    std::pair<NodeID, NodeID> getOrientedNodes() const;
};

class NavPath
{
public:
    NavPath(NavGraph *graph_, entt::entity target_, entt::registry &reg_, Traverse::TraitT traits_);

    bool buildUntil(const Connection * const con_);
    void update();
    void dump() const;

    NavGraph *m_graph;
    entt::entity m_target = entt::null;
    std::vector<ConnectionDescr> m_fullGraph;
    const Connection *m_currentTarget;
    const Traverse::TraitT m_traverseTraits;
    entt::registry &m_reg;

    std::vector<ConnectionDescr *> front;
};

struct NavSystem
{
    NavSystem(entt::registry &reg_, Application &app_, NavGraph &graph_);

    void update();
    void draw(Camera &cam_);

    std::shared_ptr<NavPath> makePath(Traverse::TraitT traverseTraits_, entt::entity goal_);

    entt::registry &m_reg;
    Renderer &m_ren;
    TextManager &m_textman;
    NavGraph &m_graph;

    std::unordered_map<Traverse::TraitT, std::weak_ptr<NavPath>> m_paths;
};

#endif

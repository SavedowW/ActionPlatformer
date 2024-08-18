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
    std::optional<ConnectionDescr *> m_nextNode;
};

class NavPath
{
public:
    NavPath(NavGraph *graph_, entt::entity target_, Traverse::TraitT traits_);

    bool buildUntil(const Connection * const con_);

    void dump() const;

    NavGraph *m_graph;
    entt::entity m_target = entt::null;

    std::vector<ConnectionDescr> m_fullGraph;

    size_t m_currentTarget;

    const Traverse::TraitT m_traverseTraits;
};

struct NavSystem
{
    NavSystem(entt::registry &reg_, Application &app_, NavGraph &graph_);

    void update();
    void draw(Camera &cam_);

    entt::registry &m_reg;
    Renderer &m_ren;
    TextManager &m_textman;
    NavGraph &m_graph;
};

#endif

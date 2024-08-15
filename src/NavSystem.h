#ifndef NAV_SYSTEM_H_
#define NAV_SYSTEM_H_
#include "Application.h"
#include "NavGraph.h"
#include "Camera.h"
#include <entt/entt.hpp>

struct ConnectionDescr
{
    Connection *m_con;
    float m_ownCost;
    float m_calculatedCost;
    std::vector<ConnectionDescr *> m_neighbourConnections;
    std::optional<ConnectionDescr *> m_nextNode;
};

class NavPath
{
public:
    NavPath(NavGraph *graph_, entt::entity target_);

    bool buildUntil(Connection *con_);

    void dump() const;

    bool m_found = true;

    NavGraph *m_graph;
    entt::entity m_target = entt::null;

    std::vector<ConnectionDescr> m_fullGraph;

    size_t m_currentTarget;
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

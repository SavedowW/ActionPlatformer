#include "NavSystem.h"
#include "CoreComponents.h"
#include <limits>

NavSystem::NavSystem(entt::registry &reg_, Application &app_, NavGraph &graph_) :
    m_reg(reg_),
    m_ren(*app_.getRenderer()),
    m_graph(graph_),
    m_textman(*app_.getTextManager())
{
}

void NavSystem::update()
{
    auto view = m_reg.view<ComponentTransform, Navigatable>();
    for (auto [idx, trans, nav] : view.each())
    {
        auto newCon = m_graph.findClosestConnection(trans.m_pos, nav.m_validTraitsOwnLocation);
        if (newCon.second <= nav.m_maxRange)
            nav.m_currentOwnConnection = newCon.first;
        else
            nav.m_currentOwnConnection = nullptr;
    }
}

void NavSystem::draw(Camera &cam_)
{
    if constexpr (gamedata::debug::drawCurrentConnection)
    {
        auto view = m_reg.view<ComponentTransform, Navigatable>();
        for (auto [idx, trans, nav] : view.each())
        {
            if (nav.m_currentOwnConnection)
            {
                auto p1 = trans.m_pos;
                auto p2 = m_graph.getConnectionCenter(nav.m_currentOwnConnection);
                m_ren.drawLine(p1, p2, {255, 150, 100, 255}, cam_);
    
                auto range = m_graph.getDistToConnection(nav.m_currentOwnConnection, trans.m_pos);
                m_textman.renderText(std::to_string(range), 2, (p1 + p2) / 2.0f - Vector2{0.0f, 12.0f}, fonts::HOR_ALIGN::CENTER, &cam_);
            }
        }
    }
}

NavPath::NavPath(NavGraph *graph_, entt::entity target_, Traverse::TraitT traits_) :
    m_graph(graph_),
    m_target(target_),
    m_fullGraph(graph_->m_connections.size()),
    m_traverseTraits(traits_)
{
    for (int i = 0 ; i < m_fullGraph.size(); ++i)
    {
        m_fullGraph[i].m_con = &m_graph->m_connections[i];
        m_fullGraph[i].m_ownCost = m_graph->m_connections[i].m_cost;
        m_fullGraph[i].m_calculatedCost = std::numeric_limits<float>::max();
        auto node1 = m_graph->m_connections[i].m_nodes[0];
        auto node2 = m_graph->m_connections[i].m_nodes[1];

        for (size_t inodeid = 0; inodeid < 2; ++inodeid)
        {
            auto nodeid = m_graph->m_connections[i].m_nodes[inodeid];
            for (auto &nb : m_graph->m_nodes[nodeid].connections)
            {
                if (!m_graph->m_connections[nb].isOnNodes(m_graph->m_connections[i].m_nodes[0], m_graph->m_connections[i].m_nodes[1]))
                    m_fullGraph[i].m_neighbourConnections.push_back(&m_fullGraph[m_graph->m_connections[nb].m_ownId]);
            }
        }
        
    }
}

bool NavPath::buildUntil(Connection *con_)
{
    m_fullGraph[m_currentTarget].m_calculatedCost = m_fullGraph[m_currentTarget].m_ownCost;
    std::vector<ConnectionDescr *> front{&m_fullGraph[m_currentTarget]};

    while (!front.empty())
    {
        std::sort(front.begin(), front.end(), [](const ConnectionDescr *c1_, const ConnectionDescr *c2_){return c1_->m_calculatedCost < c2_->m_calculatedCost;});
        auto used = front.front();
        front.erase(front.begin());
        bool found = false;
        for (auto *con : used->m_neighbourConnections)
        {
            auto newcost = con->m_ownCost + used->m_calculatedCost;
            size_t orientation = (used->m_con->m_nodes[1] == con->m_con->m_nodes[0] || used->m_con->m_nodes[1] == con->m_con->m_nodes[1] ? 0 : 1);
            if (newcost < con->m_calculatedCost && Traverse::canTraverseByPath(m_traverseTraits, used->m_con->m_traverses[orientation]))
            {
                std::cout << "Editing " << con->m_con->m_ownId << ": " << con->m_calculatedCost << " => " << newcost << std::endl;
                front.push_back(con);
                con->m_calculatedCost = newcost;
                con->m_nextNode = used;

                if (con->m_con == con_)
                    found = true;
            }
        }

        if (found)
            return true;
    }

    std::cout << "Failed to find path\n";
    m_fullGraph[con_->m_ownId].m_nextNode = nullptr;
    return false;
}

void NavPath::dump() const
{
    for (const auto &el : m_fullGraph)
    {
        std::cout << el.m_con->m_ownId << " (" << el.m_con->m_nodes[0] << ", " << el.m_con->m_nodes[1] << ") " << el.m_ownCost << " / " << el.m_calculatedCost;
        if (el.m_nextNode.has_value())
        {
            if (*el.m_nextNode)
                std::cout << " -> " << el.m_nextNode.value()->m_con->m_ownId;
            else
                std::cout << " NOT FOUND";
        }
        std::cout << std::endl;
    }
}

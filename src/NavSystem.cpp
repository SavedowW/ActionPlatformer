#include "NavSystem.h"
#include "CoreComponents.h"
#include "Profile.h"
#include "GameData.h"
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
    PROFILE_FUNCTION;

    const auto view = m_reg.view<ComponentTransform, Navigatable>();
    for (const auto [idx, trans, nav] : view.each())
    {
        if (nav.m_checkIfGrounded)
            if (m_reg.get<ComponentPhysical>(idx).m_onGround == entt::null)
                continue;

        auto newCon = m_graph.findClosestConnection(trans.m_pos, nav.m_validTraitsOwnLocation);
        if (newCon.second <= nav.m_maxRange)
            nav.m_currentOwnConnection = newCon.first;
        else
            nav.m_currentOwnConnection = nullptr;
    }

    for (auto &path : m_paths)
    {
        if (!path.second.expired())
            path.second.lock()->update();
    }
}

void NavSystem::draw(Camera &cam_)
{
    if constexpr (gamedata::debug::drawCurrentConnection)
    {
        const auto view = m_reg.view<ComponentTransform, Navigatable>();
        for (const auto [idx, trans, nav] : view.each())
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

    if constexpr (gamedata::debug::debugPathDisplay)
    {
        auto ipath = m_paths.find(gamedata::debug::debugPathDisplay);
        if (ipath != m_paths.end() && !ipath->second.expired())
        {
            auto path = ipath->second.lock().get();
            for (const auto &con : path->m_fullGraph)
            {
                if (path->m_currentTarget == con.m_con)
                {
                    auto origin = m_graph.getNodePos(con.m_con->m_nodes[0]) - Vector2{1.0f, 1.0f};
                    auto tar = m_graph.getNodePos(con.m_con->m_nodes[1]) - Vector2{1.0f, 1.0f};
                    m_ren.drawLine(origin, tar, {0, 255, 50, 200}, cam_);
                }
                else if (con.m_nextConnection.has_value())
                {

                    auto oriented = con.getOrientedNodes();
                    if (*con.m_nextConnection)
                    {
                        auto origin = m_graph.getNodePos(oriented.first) - Vector2{1.0f, 1.0f};
                        auto tar = m_graph.getNodePos(oriented.second) - Vector2{1.0f, 1.0f};
                        auto delta = tar - origin;
                        auto center = tar - delta  / 4.0f;
                        m_ren.drawLine(center, tar, {0, 255, 0, 200}, cam_);
                    }
                    else
                    {
                        m_ren.drawLine(
                            m_graph.getNodePos(oriented.first) - Vector2{1.0f, 1.0f}, 
                            m_graph.getNodePos(oriented.second) - Vector2{1.0f, 1.0f}, 
                            {255, 0, 0, 200}, cam_);
                    }
                }
            }
        }
    }
}

std::shared_ptr<NavPath> NavSystem::makePath(Traverse::TraitT traverseTraits_, entt::entity goal_)
{
    auto found = m_paths.find(traverseTraits_);
    if (found == m_paths.end())
    {
        auto newpath = std::shared_ptr<NavPath>(new NavPath(&m_graph, goal_, m_reg, traverseTraits_));
        m_paths[traverseTraits_] = newpath;
        return newpath;
    }
    else
    {
        if (found->second.expired())
        {
            auto newpath = std::shared_ptr<NavPath>(new NavPath(&m_graph, goal_, m_reg, traverseTraits_));
            found->second = newpath;
            return newpath;
        }
        else
        {
            return found->second.lock();
        }
    }
}

NavPath::NavPath(NavGraph *graph_, entt::entity target_, entt::registry &reg_, Traverse::TraitT traits_) :
    m_graph(graph_),
    m_target(target_),
    m_fullGraph(graph_->m_connections.size()),
    m_traverseTraits(traits_),
    m_reg(reg_)
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

    m_currentTarget = reg_.get<Navigatable>(target_).m_currentOwnConnection;
}

bool NavPath::buildUntil(const Connection * const con_)
{
    if (m_fullGraph[con_->m_ownId].m_nextConnection.has_value())
        return *m_fullGraph[con_->m_ownId].m_nextConnection;

    if (!m_currentTarget)
    {
        m_fullGraph[con_->m_ownId].m_nextConnection = nullptr;
        m_fullGraph[con_->m_ownId].m_nextNode = 0;
        return false;
    }

    m_fullGraph[m_currentTarget->m_ownId].m_nextConnection = &m_fullGraph[m_currentTarget->m_ownId];

    if (con_ == m_currentTarget)
        return true;

    m_fullGraph[m_currentTarget->m_ownId].m_calculatedCost = m_fullGraph[m_currentTarget->m_ownId].m_ownCost;

    while (!front.empty())
    {
        std::sort(front.begin(), front.end(), [](const ConnectionDescr *c1_, const ConnectionDescr *c2_){return c1_->m_calculatedCost < c2_->m_calculatedCost;});
        auto used = front.front();
        front.erase(front.begin());
        bool found = false;
        for (auto *con : used->m_neighbourConnections)
        {
            auto newcost = con->m_ownCost + used->m_calculatedCost;
            size_t orientation = (con->m_con->m_nodes[1] == used->m_con->m_nodes[0] || con->m_con->m_nodes[1] == used->m_con->m_nodes[1] ? 0 : 1);
            if (newcost < con->m_calculatedCost && Traverse::canTraverseByPath(m_traverseTraits, con->m_con->m_traverses[orientation]))
            {
                //std::cout << "Editing " << con->m_con->m_ownId << ": " << con->m_calculatedCost << " => " << newcost << std::endl;
                front.push_back(con);
                con->m_calculatedCost = newcost;
                con->m_nextConnection = used;
                con->m_nextNode = 1 - orientation;

                if (con->m_con == con_)
                    found = true;
            }
        }

        if (found)
            return true;
    }

    std::cout << "Failed to find path\n";
    m_fullGraph[con_->m_ownId].m_nextConnection = nullptr;
    m_fullGraph[con_->m_ownId].m_nextNode = 0;
    return false;
}

void NavPath::update()
{
    auto newtar = m_reg.get<Navigatable>(m_target).m_currentOwnConnection;
    if (newtar != m_currentTarget)
    {
        for (auto &el : m_fullGraph)
        {
            el.m_calculatedCost = std::numeric_limits<float>::max();
            el.m_nextConnection.reset();
        }
        m_currentTarget = newtar;
        if (m_currentTarget)
            front = {&m_fullGraph[m_currentTarget->m_ownId]};
        else
            front.clear();
    }
}

void NavPath::dump() const
{
    for (const auto &el : m_fullGraph)
    {
        std::cout << el.m_con->m_ownId << " (" << el.m_con->m_nodes[0] << ", " << el.m_con->m_nodes[1] << ") " << el.m_ownCost << " / " << el.m_calculatedCost;
        if (el.m_nextConnection.has_value())
        {
            if (*el.m_nextConnection)
                std::cout << " -> " << el.m_nextConnection.value()->m_con->m_ownId;
            else
                std::cout << " NOT FOUND";
        }
        std::cout << std::endl;
    }
}

std::pair<NodeID, NodeID> ConnectionDescr::getOrientedNodes() const
{
    if (!m_nextConnection.has_value() || !(*m_nextConnection))
        return {m_con->m_nodes[0], m_con->m_nodes[1]};

    if (m_con->m_nodes[1] == (*m_nextConnection)->m_con->m_nodes[0] || m_con->m_nodes[1] == (*m_nextConnection)->m_con->m_nodes[1])
        return {m_con->m_nodes[0], m_con->m_nodes[1]};
    else
        return {m_con->m_nodes[1], m_con->m_nodes[0]};
}

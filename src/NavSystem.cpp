#include "NavSystem.h"
#include "CoreComponents.h"
#include "Profile.h"
#include "Configuration.h"
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
    if (ConfigurationManager::instance().m_debug.m_drawCurrentConnection)
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

    if (ConfigurationManager::instance().m_debug.m_debugPathDisplay)
    {
        auto ipath = m_paths.find(ConfigurationManager::instance().m_debug.m_debugPathDisplay);
        if (ipath != m_paths.end() && !ipath->second.expired())
        {
            auto path = ipath->second.lock().get();
            for (const auto &con : path->m_graphView)
            {
                if (path->isTargetConnection(con.second.m_originalCon.m_ownId))
                {
                    auto origin = m_graph.getNodePos(con.second.m_originalCon.m_nodes[0]) - Vector2{1.0f, 1.0f};
                    auto tar = m_graph.getNodePos(con.second.m_originalCon.m_nodes[1]) - Vector2{1.0f, 1.0f};
                    m_ren.drawLine(origin, tar, {0, 255, 50, 200}, cam_);
                }
                else if (con.second.m_nextConnection.has_value())
                {

                    auto oriented = con.second.getOrientedNodes();
                    if (*con.second.m_nextConnection)
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
        auto newpath = std::shared_ptr<NavPath>(new NavPath(m_graph, goal_, m_reg, traverseTraits_));
        m_paths[traverseTraits_] = newpath;
        return newpath;
    }
    else
    {
        if (found->second.expired())
        {
            auto newpath = std::shared_ptr<NavPath>(new NavPath(m_graph, goal_, m_reg, traverseTraits_));
            found->second = newpath;
            return newpath;
        }
        else
        {
            return found->second.lock();
        }
    }
}

NavPath::NavPath(const NavGraph &graph_, entt::entity target_, entt::registry &reg_, Traverse::TraitT traits_) :
    m_graph(graph_),
    m_target(target_),
    m_traverseTraits(traits_),
    m_reg(reg_)
{
    for (const auto &el : m_graph.m_connections)
    {
        ConnectionDescr conDescr(el);
        
        m_graphView.emplace(el.m_ownId, std::move(conDescr));
        
    }

    // For each connection description, find all neighbours except it's equivalents (on the same pair of nodes)
    for (auto &conDescr : m_graphView)
    {
        const auto &nodeIds = conDescr.second.m_originalCon.m_nodes;

        for (const auto &nodeid : nodeIds)
        {
            for (const auto &nb : graph_.m_nodes[nodeid].connections)
            {
                if (!graph_.m_connections[nb].isOnNodes(nodeIds[0], nodeIds[1]))
                    conDescr.second.m_neighbourConnections.push_back(&m_graphView.at(nb));
            }
        }
    }

    m_currentTarget = nullptr;
}

bool NavPath::buildUntil(const Connection * const con_)
{
    // If path was already calculated, return it's status
    if (m_graphView.at(con_->m_ownId).m_nextConnection.has_value())
        return *m_graphView.at(con_->m_ownId).m_nextConnection;

    // If there is no target, there is no path
    //TODO - move to the moment m_currentTarget is set
    if (!m_currentTarget)
    {
        m_graphView.at(con_->m_ownId).m_nextConnection = nullptr;
        m_graphView.at(con_->m_ownId).m_nextNode = 0;
        return false;
    }

    m_graphView.at(m_currentTarget->m_ownId).m_nextConnection = &m_graphView.at(m_currentTarget->m_ownId);

    if (con_ == m_currentTarget)
        return true;

    m_graphView.at(m_currentTarget->m_ownId).m_calculatedCost = m_graphView.at(m_currentTarget->m_ownId).m_originalCon.m_cost;

    while (!m_front.empty())
    {
        bool requireSort = false;

        for (int i = 0; i < m_front.size(); ++i)
            for (int k = i + 1; k < m_front.size(); ++k)
                if (m_front[i] == m_front[k])
                    std::cout << "Duplicated!" << std::endl;

        auto used = m_front.front();
        m_front.erase(m_front.begin());
        bool found = false;
        for (auto *con : used->m_neighbourConnections)
        {
            const auto newcost = con->m_originalCon.m_cost + used->m_calculatedCost;
            size_t orientation = (con->m_originalCon.m_nodes[1] == used->m_originalCon.m_nodes[0] || con->m_originalCon.m_nodes[1] == used->m_originalCon.m_nodes[1] ? 0 : 1);

            // TODO: move traverse type check to the graph constructor
            if (newcost < con->m_calculatedCost && Traverse::canTraverseByPath(m_traverseTraits, con->m_originalCon.m_traverses[orientation]))
            {
                //std::cout << "Editing " << con->m_con->m_ownId << ": " << con->m_calculatedCost << " => " << newcost << std::endl;
                con->m_calculatedCost = newcost;
                con->m_nextConnection = used;
                con->m_nextNode = 1 - orientation;
                
                if (&con->m_originalCon == con_)
                    found = true;

                m_front.push_back(con);

                requireSort = true;
            }
        }

        // TODO: duplicates?
        if (requireSort)
            std::sort(m_front.begin(), m_front.end(), [](const ConnectionDescr *c1_, const ConnectionDescr *c2_){return c1_->m_calculatedCost < c2_->m_calculatedCost;});

        if (found)
            return true;
    }

    std::cout << "Failed to find path\n";
    m_graphView.at(con_->m_ownId).m_nextConnection = nullptr;
    m_graphView.at(con_->m_ownId).m_nextNode = 0;
    return false;
}

void NavPath::update()
{
    auto newtar = m_reg.get<Navigatable>(m_target).m_currentOwnConnection;
    if (newtar != m_currentTarget)
    {
        for (auto &el : m_graphView)
        {
            el.second.m_calculatedCost = std::numeric_limits<float>::max();
            el.second.m_nextConnection.reset();
        }
        m_currentTarget = newtar;
        if (m_currentTarget)
            m_front = {&m_graphView.at(m_currentTarget->m_ownId)};
        else
            m_front.clear();
    }
}

void NavPath::dump() const
{
    for (const auto &el : m_graphView)
    {
        std::cout << el.second.m_originalCon.m_ownId << " (" << el.second.m_originalCon.m_nodes[0] << ", " << el.second.m_originalCon.m_nodes[1] << ") " << el.second.m_originalCon.m_cost << " / " << el.second.m_calculatedCost;
        if (el.second.m_nextConnection.has_value())
        {
            if (*el.second.m_nextConnection)
                std::cout << " -> " << el.second.m_nextConnection.value()->m_originalCon.m_ownId;
            else
                std::cout << " NOT FOUND";
        }
        std::cout << std::endl;
    }
}

bool NavPath::isTargetConnection(ConnectionID id_) const
{
    return m_currentTarget && m_currentTarget->m_ownId == id_;
}

ConnectionDescr::ConnectionDescr(const Connection&con_) :
    m_originalCon(con_),
    m_calculatedCost(std::numeric_limits<float>::max())
{
}

std::pair<NodeID, NodeID> ConnectionDescr::getOrientedNodes() const
{
    if (!m_nextConnection.has_value() || !(*m_nextConnection))
        return {m_originalCon.m_nodes[0], m_originalCon.m_nodes[1]};

    if (m_originalCon.m_nodes[1] == (*m_nextConnection)->m_originalCon.m_nodes[0] || m_originalCon.m_nodes[1] == (*m_nextConnection)->m_originalCon.m_nodes[1])
        return {m_originalCon.m_nodes[0], m_originalCon.m_nodes[1]};
    else
        return {m_originalCon.m_nodes[1], m_originalCon.m_nodes[0]};
}

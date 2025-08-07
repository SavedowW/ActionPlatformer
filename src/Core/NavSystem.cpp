#include "NavSystem.h"
#include "CoreComponents.h"
#include "Profile.h"
#include "Configuration.h"
#include <limits>

NavSystem::NavSystem(entt::registry &reg_, Application &app_, NavGraph &graph_) :
    m_reg(reg_),
    m_ren(app_.getRenderer()),
    m_graph(graph_),
    m_textman(app_.getTextManager())
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

        auto newCon = m_graph.findClosestConnection(trans.m_pos, nav.m_traverseTraits);
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
                m_ren.drawCircleOutline(trans.m_pos, nav.m_maxRange, {0, 255, 50, 200}, cam_);

                const auto p1 = trans.m_pos;
                const auto p2 = m_graph.getConnectionCenter(*nav.m_currentOwnConnection);
                m_ren.drawLine(p1, p2, {255, 150, 100, 255}, cam_);
    
                const auto range = m_graph.getDistToConnection(*nav.m_currentOwnConnection, trans.m_pos);
                m_textman.renderText(std::to_string(range), 2, (p1 + p2) / 2.0f - Vector2{0.0f, 12.0f}, fonts::HOR_ALIGN::CENTER, &cam_);
            }
            else
                m_ren.drawCircleOutline(trans.m_pos, nav.m_maxRange, {255, 150, 100, 200}, cam_);
        }
    }

    if (ConfigurationManager::instance().m_debug.m_debugPathDisplay)
    {
        const auto ipath = m_paths.find(ConfigurationManager::instance().m_debug.m_debugPathDisplay);
        if (ipath != m_paths.end() && !ipath->second.expired())
        {
            const auto &path = *ipath->second.lock().get();
            for (const auto &con : path.m_graphView)
            {
                if (path.isTargetConnection(con.second.m_originalCon.m_ownId))
                {
                    const auto origin = m_graph.getNodePos(con.second.m_originalCon.m_nodes[0]) - Vector2{1.0f, 1.0f};
                    const auto tar = m_graph.getNodePos(con.second.m_originalCon.m_nodes[1]) - Vector2{1.0f, 1.0f};
                    m_ren.drawLine(origin, tar, {0, 255, 50, 200}, cam_);
                }
                else
                {
                    const auto oriented = con.second.getOrientedNodes();
                    switch (con.second.getStatus())
                    {
                        case ConnectionDescr::Status::FOUND:
                        {
                            const auto origin = m_graph.getNodePos(oriented.first) - Vector2{1.0f, 1.0f};
                            const auto tar = m_graph.getNodePos(oriented.second) - Vector2{1.0f, 1.0f};
                            const auto delta = tar - origin;
                            const auto center = tar - delta  / 4.0f;
                            m_ren.drawLine(center, tar, {0, 255, 0, 200}, cam_);
                        }
                            break;

                        case ConnectionDescr::Status::NOT_EXISTS:
                            m_ren.drawLine(
                                m_graph.getNodePos(oriented.first) - Vector2{1.0f, 1.0f}, 
                                m_graph.getNodePos(oriented.second) - Vector2{1.0f, 1.0f}, 
                                {255, 0, 0, 200}, cam_);
                            break;
                    }
                }
            }

            const auto tarPos = m_reg.get<ComponentTransform>(path.m_target).m_pos;
            
            if (path.m_currentTarget)
            {
                const auto p2 = m_graph.getConnectionCenter(*path.m_currentTarget);
                const auto range = m_graph.getDistToConnection(*path.m_currentTarget, tarPos);
                
                m_ren.drawCircleOutline(tarPos, path.m_targetMaxConnectionRange, {0, 255, 50, 200}, cam_);
                
                m_ren.drawLine(tarPos, p2, {255, 150, 100, 255}, cam_);
                m_textman.renderText(std::to_string(range), 2, (tarPos + p2) / 2.0f - Vector2{0.0f, 12.0f}, fonts::HOR_ALIGN::CENTER, &cam_);
            }
            else
                m_ren.drawCircleOutline(tarPos, path.m_targetMaxConnectionRange, {255, 150, 100, 200}, cam_);
        }
    }
}

NavPath::Follower NavSystem::makePath(Traverse::TraitT traverseTraits_, entt::entity goal_, float maxTarRange_)
{
    auto found = m_paths.find(traverseTraits_);
    if (found == m_paths.end())
    {
        auto newpath = std::shared_ptr<NavPath>(new NavPath(m_graph, goal_, m_reg, traverseTraits_, maxTarRange_));
        m_paths[traverseTraits_] = newpath;
        return NavPath::Follower{newpath};
    }
    else
    {
        if (found->second.expired())
        {
            auto newpath = std::shared_ptr<NavPath>(new NavPath(m_graph, goal_, m_reg, traverseTraits_, maxTarRange_));
            found->second = newpath;
            return NavPath::Follower{newpath};
        }
        else
        {
            return NavPath::Follower{found->second.lock()};
        }
    }
}

NavPath::NavPath(const NavGraph &graph_, entt::entity target_, entt::registry &reg_, Traverse::TraitT traits_, float targetMaxConnectionRange_) :
    m_graph(graph_),
    m_target(target_),
    m_targetMaxConnectionRange(targetMaxConnectionRange_),
    m_traverseTraits(traits_),
    m_reg(reg_)
{
    for (const auto &el : m_graph.m_connections)
    {
        if (!Traverse::canTraverseByPath(m_traverseTraits, el.m_traverses[0]) &&
            !Traverse::canTraverseByPath(m_traverseTraits, el.m_traverses[1]))
            continue;

        ConnectionDescr conDescr(el);
        conDescr.setNoPathFound();
        
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

    updateTarget();
}

NavPath::Status NavPath::buildUntil(const Connection * const con_)
{
    if (!m_currentTarget)
        return NavPath::Status::NOT_FOUND;

    if (con_->m_ownId == m_currentTarget->m_ownId)
        return NavPath::Status::FINISHED;

    // If path was already calculated, return it's status
    if (m_graphView.at(con_->m_ownId).m_nextConnection.has_value())
        return (*m_graphView.at(con_->m_ownId).m_nextConnection ? NavPath::Status::FOUND : NavPath::Status::NOT_FOUND);

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
                con->setPathFound(used, newcost, 1 - orientation);
                
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
            return NavPath::Status::FOUND;
    }

    std::cout << "Failed to find path\n";
    m_graphView.at(con_->m_ownId).setNoPathFound();
    return NavPath::Status::NOT_FOUND;
}

void NavPath::update()
{
    updateTarget();
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

void NavPath::updateTarget()
{
    const Connection *newtar = nullptr;
    auto newCon = m_graph.findClosestConnection(m_reg.get<ComponentTransform>(m_target).m_pos, m_traverseTraits);
        if (newCon.second <= m_targetMaxConnectionRange)
            newtar = newCon.first;
        else
            newtar = nullptr;

    if (newtar != m_currentTarget)
    {
        m_currentTarget = newtar;

        if (m_currentTarget)
        {
            for (auto &el : m_graphView)
                el.second.resetResults();
            m_front = {&m_graphView.at(m_currentTarget->m_ownId)};

            m_graphView.at(m_currentTarget->m_ownId).m_calculatedCost = m_graphView.at(m_currentTarget->m_ownId).m_originalCon.m_cost;
        }
        else
        {
            for (auto &el : m_graphView)
                el.second.setNoPathFound();
            m_front.clear();
        }
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

void ConnectionDescr::resetResults()
{
    m_nextConnection.reset();
    m_calculatedCost = std::numeric_limits<float>::max();
}

void ConnectionDescr::setPathFound(const ConnectionDescr *con_, float calculatedCost_, int nextNode_)
{
    assert(con_);
    m_nextConnection = con_;
    m_calculatedCost = calculatedCost_;
    m_nextNode = nextNode_;
}

void ConnectionDescr::setNoPathFound()
{
    m_nextConnection = nullptr;
}

ConnectionDescr::Status ConnectionDescr::getStatus() const
{
    if (m_nextConnection.has_value())
    {
        if (*m_nextConnection)
            return Status::FOUND;
        else
            return Status::NOT_EXISTS;
    }
    else
        return Status::UNRESOLVED;
}

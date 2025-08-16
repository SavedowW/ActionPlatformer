#include "NavGraph.h"
#include "Application.h"
#include "Configuration.h"

NavGraph::NavGraph() :
    m_ren(Application::instance().m_renderer),
    m_textman(Application::instance().m_textManager)
{
}

NodeID NavGraph::makeNode(const Vector2<float> &pos_)
{
    m_nodes.emplace_back(pos_);
    return m_nodes.size() - 1;
}

ConnectionID NavGraph::makeConnection(NodeID node1_, NodeID node2_, Traverse::TraitT traverseTo2_, Traverse::TraitT traverseTo1_)
{
    m_connections.emplace_back(node1_, node2_, traverseTo2_, traverseTo1_, (m_nodes[node1_].m_position - m_nodes[node2_].m_position).length(), m_connections.size());
    m_nodes[node1_].connections.push_back(m_connections.size() - 1);
    m_nodes[node2_].connections.push_back(m_connections.size() - 1);

    return m_connections.size() - 1;
}

std::pair<const Connection *, float> NavGraph::findClosestConnection(const Vector2<float> &pos_, Traverse::TraitT options_) const
{
    const Connection *mincon = nullptr;
    float mindst = 0.0f;

    for (const auto &con : m_connections)
    {
        if (Traverse::canTraverseByPath(options_, con.m_traverses[0]) || Traverse::canTraverseByPath(options_, con.m_traverses[1]))
        {
            auto res = utils::distToLineSegment(m_nodes[con.m_nodes[0]].m_position, m_nodes[con.m_nodes[1]].m_position, pos_);
            if (!mincon || res < mindst)
            {
                mincon = &con;
                mindst = res;
            }
        }
    }

    return {mincon, mindst};

}

void NavGraph::draw(Camera &cam_)
{
    if (ConfigurationManager::instance().m_debug.m_drawNavGraph)
    {
        const Vector2<float> nodeSize{5.0f, 5.0f};
        for (size_t i = 0; i < m_nodes.size(); ++i)
        {
            auto &node = m_nodes[i];
            m_ren.drawRectangle(node.m_position - nodeSize / 2.0f, nodeSize, {255, 127, 39, 255}, cam_);
            m_textman.renderText(std::to_string(i), 2, node.m_position - Vector2{0.0f, 12.0f}, fonts::HOR_ALIGN::CENTER, &cam_);
        }
    
        for (const auto &con : m_connections)
        {
            m_ren.drawLine(m_nodes[con.m_nodes[0]].m_position - Vector2{1.0f, 1.0f}, m_nodes[con.m_nodes[1]].m_position - Vector2{1.0f, 1.0f}, {255, 127, 39, 200}, cam_);
    
            auto center = (m_nodes[con.m_nodes[0]].m_position + m_nodes[con.m_nodes[1]].m_position) / 2.0f;
            m_textman.renderText(std::to_string(con.m_ownId), 2, center - Vector2{0.0f, 12.0f}, fonts::HOR_ALIGN::CENTER, &cam_);
        }
    }
}

Vector2<float> NavGraph::getConnectionCenter(const Connection &con_) const
{
    auto p1 = m_nodes[con_.m_nodes[0]].m_position;
    auto p2 = m_nodes[con_.m_nodes[1]].m_position;
    return (p1 + p2) / 2.0f;
}

float NavGraph::getDistToConnection(const Connection &con_, const Vector2<float> &pos_) const
{
    return utils::distToLineSegment(m_nodes[con_.m_nodes[0]].m_position, m_nodes[con_.m_nodes[1]].m_position, pos_);
}

const Connection &NavGraph::getConnection(ConnectionID con_) const
{
    return m_connections[con_];
}

Vector2<float> NavGraph::getNodePos(NodeID nd_) const
{
    return m_nodes[nd_].m_position;
}

Connection::Connection(NodeID node1_, NodeID node2_, Traverse::TraitT traverseTo2_, Traverse::TraitT traverseTo1_, float cost_, ConnectionID ownId_) :
    m_nodes{node1_, node2_},
    m_traverses{traverseTo2_, traverseTo1_},
    m_cost(cost_),
    m_ownId{ownId_}
{
}

bool Connection::isOnNodes(NodeID nd1_, NodeID nd2_) const
{
    return (m_nodes[0] == nd1_ && m_nodes[1] == nd2_) || (m_nodes[0] == nd2_ && m_nodes[1] == nd1_);
}

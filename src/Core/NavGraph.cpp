#include "NavGraph.h"
#include <algorithm>

NodeID NavGraph::makeNode(const Vector2<float> &pos_)
{
    m_nodes.emplace_back(pos_);
    return m_nodes.size() - 1;
}

ConnectionID NavGraph::makeConnection(NodeID node1_, NodeID node2_, const std::set<TraverseTraitT> &traverse_)
{
    m_connections.emplace_back(node1_, node2_, traverse_);
    m_nodes[node1_].connections.push_back(&m_connections.back());
    m_nodes[node2_].connections.push_back(&m_connections.back());

    return m_connections.size() - 1;
}

Connection *NavGraph::findClosestConnection(const Vector2<float> &pos_, const std::set<TraverseTraitT> &options_)
{
    Connection *mincon = nullptr;
    float mindst = 0.0f;

    for (auto &con : m_connections)
    {
        bool hasOverlap = std::any_of(con.m_traverseTypes.begin(), con.m_traverseTypes.end(), [&options_](auto &elem){return options_.contains(elem);});
        if (hasOverlap)
        {
            auto res = utils::distToLineSegment(m_nodes[con.m_node1].m_position, m_nodes[con.m_node2].m_position, pos_);
            if (!mincon || res < mindst)
            {
                mincon = &con;
                mindst = res;
            }
        }
    }

    return mincon;

}

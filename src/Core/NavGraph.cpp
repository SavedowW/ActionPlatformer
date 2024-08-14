#include "NavGraph.h"
#include <algorithm>

NodeID NavGraph::makeNode(const Vector2<float> &pos_)
{
    m_nodes.emplace_back(pos_);
    return m_nodes.size() - 1;
}

ConnectionID NavGraph::makeConnection(NodeID node1_, NodeID node2_, const std::set<TraverseTraitT> &traverseTo2_, const std::set<TraverseTraitT> &traverseTo1_)
{
    m_connections.emplace_back(node1_, node2_, traverseTo2_, traverseTo1_);
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
        bool hasOverlap = std::any_of(options_.begin(), options_.end(), [&traverses = con.m_traverses](auto &elem){return traverses[0].contains(elem) || traverses[1].contains(elem);});
        if (hasOverlap)
        {
            auto res = utils::distToLineSegment(m_nodes[con.m_nodes[0]].m_position, m_nodes[con.m_nodes[1]].m_position, pos_);
            if (!mincon || res < mindst)
            {
                mincon = &con;
                mindst = res;
            }
        }
    }

    return mincon;

}

void NavGraph::draw(Renderer &ren_, Camera &cam_)
{
    const Vector2<float> nodeSize{5.0f, 5.0f};
    for (auto &node : m_nodes)
    {
        ren_.drawRectangle(node.m_position - nodeSize / 2.0f, nodeSize, {255, 127, 39, 255}, cam_);
    }

    for (auto &con : m_connections)
    {
        ren_.drawLine(m_nodes[con.m_nodes[0]].m_position - Vector2{1.0f, 1.0f}, m_nodes[con.m_nodes[1]].m_position - Vector2{1.0f, 1.0f}, {255, 127, 39, 100}, cam_);
    }
}

Connection::Connection(NodeID node1_, NodeID node2_, const std::set<TraverseTraitT> &traverseTo2_, const std::set<TraverseTraitT> &traverseTo1_) :
    m_nodes{node1_, node2_},
    m_traverses{traverseTo2_, traverseTo1_}
{
}

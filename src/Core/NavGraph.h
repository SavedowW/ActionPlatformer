#ifndef NAV_GRAPH_H_
#define NAV_GRAPH_H_
#include "Vector2.h"
#include <cstdint>
#include <set>
#include <vector>

class Node;

using TraverseTraitT = uint32_t;
using NodeID = size_t;
using ConnectionID = size_t;

struct Connection
{
    NodeID m_node1;
    NodeID m_node2;
    std::set<TraverseTraitT> m_traverseTypes;
};

struct Node
{
    Vector2<float> m_position;
    std::vector<Connection*> connections;
};

class NavGraph
{
public:
    NodeID makeNode(const Vector2<float> &pos_);
    ConnectionID makeConnection(NodeID node1_, NodeID node2_, const std::set<TraverseTraitT> &traverse_);
    Connection *findClosestConnection(const Vector2<float> &pos_, const std::set<TraverseTraitT> &options_);

private:
    std::vector<Node> m_nodes;
    std::vector<Connection> m_connections;

};

#endif
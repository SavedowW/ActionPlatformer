#ifndef NAV_GRAPH_H_
#define NAV_GRAPH_H_
#include "Vector2.h"
#include "Renderer.h"
#include "Camera.h"
#include <cstdint>
#include <set>
#include <vector>

class Node;

using TraverseTraitT = uint32_t;
using NodeID = size_t;
using ConnectionID = size_t;

struct Connection
{
    Connection(NodeID node1_, NodeID node2_, const std::set<TraverseTraitT> &traverseTo2_, const std::set<TraverseTraitT> &traverseTo1_);
    NodeID m_nodes[2];
    std::set<TraverseTraitT> m_traverses[2];
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
    ConnectionID makeConnection(NodeID node1_, NodeID node2_, const std::set<TraverseTraitT> &traverseTo2_, const std::set<TraverseTraitT> &traverseTo1_);
    Connection *findClosestConnection(const Vector2<float> &pos_, const std::set<TraverseTraitT> &options_);

    void draw(Renderer &ren_, Camera &cam_);

private:
    std::vector<Node> m_nodes;
    std::vector<Connection> m_connections;

};

#endif
#ifndef NAV_GRAPH_H_
#define NAV_GRAPH_H_
#include "Vector2.h"
#include "Application.h"
#include "Camera.h"
#include <cstdint>
#include <set>
#include <vector>

class Node;

using TraverseTraitT = uint32_t;
using NodeID = size_t;
using ConnectionID = size_t;

template<typename... TraitsT>
constexpr std::set<TraverseTraitT> makeTraitList(TraitsT... traits_)
{
    std::set<TraverseTraitT> retval;
    (retval.insert(static_cast<TraverseTraitT>(traits_)), ...);

    return retval;
}

struct Connection
{
    Connection(NodeID node1_, NodeID node2_, const std::set<TraverseTraitT> &traverseTo2_, const std::set<TraverseTraitT> &traverseTo1_, bool requireFallthroughTo2_, bool requireFallthroughTo1_, float cost_, ConnectionID ownId_);
    NodeID m_nodes[2];
    std::set<TraverseTraitT> m_traverses[2];
    bool m_requireFallthrough[2] = {false, false};
    float m_cost = 0.0f;
    ConnectionID m_ownId;

    bool isOnNodes(NodeID nd1_, NodeID nd2_) const;
};

struct Node
{
    Vector2<float> m_position;
    std::vector<ConnectionID> connections;
};

class NavGraph
{
public:
    NavGraph(Application &app_);

    NodeID makeNode(const Vector2<float> &pos_);
    ConnectionID makeConnection(NodeID node1_, NodeID node2_, const std::set<TraverseTraitT> &traverseTo2_, const std::set<TraverseTraitT> &traverseTo1_,
        bool requireFallthroughTo2_, bool requireFallthroughTo1_);
    std::pair<Connection *, float> findClosestConnection(const Vector2<float> &pos_, const std::set<TraverseTraitT> &options_);

    void draw(Camera &cam_);
    Vector2<float> getConnectionCenter(const Connection *con_) const;
    float getDistToConnection(const Connection *con_, const Vector2<float> &pos_);

    void setcost(ConnectionID id_, float cost_);

private:
    std::vector<Node> m_nodes;
    std::vector<Connection> m_connections;

    Renderer &m_ren;
    TextManager &m_textman;

    friend class NavPath;

};

#endif
#ifndef NAV_GRAPH_H_
#define NAV_GRAPH_H_
#include "Renderer.h"
#include "TextManager.h"
#include "Vector2.hpp"
#include "Camera.h"
#include <cstdint>
#include <vector>

struct Node;

using NodeID = size_t;
using ConnectionID = size_t;

namespace Traverse
{
    // Describes traits necessary to be on or move through a given connection
    using TraitT = uint32_t;
    inline constexpr TraitT ReservedBits = 1;
    inline constexpr TraitT FallthroughBitID = 0;
    inline constexpr TraitT ReservedMask = 1;
    inline constexpr TraitT FreeMask = ~ReservedMask;

    template<typename... TraitsT>
    constexpr TraitT makeSignature(bool allowFallthrough_, TraitsT... traits_)
    {
        TraitT signature{0};
        signature |= (static_cast<TraitT>(allowFallthrough_ << FallthroughBitID));

        if constexpr (sizeof...(TraitsT) > 0)
            return signature | ((static_cast<TraitT>(traits_) | ...) << ReservedBits);
        else
            return signature;
    }

    template<typename... TraitsT>
    constexpr TraitT extendSignature(TraitT signature_, TraitsT... traits_)
    {
        if constexpr (sizeof...(TraitsT) > 0)
            return signature_ | ((static_cast<TraitT>(traits_) | ...) << ReservedBits);
        else
            return signature_;
    }

    constexpr bool canTraverseByPath(const TraitT &traverse_, const TraitT &path_)
    {
        auto required = path_ & ReservedMask;
        return ((traverse_ & required) == required) &&
            (traverse_ & path_ & FreeMask);
    }

    constexpr bool compareSignaturesOnlyFree(const TraitT &t1_, const TraitT &t2_)
    {
        return (t1_ & t2_ & FreeMask);
    }
}

struct Connection
{
    Connection(NodeID node1_, NodeID node2_, Traverse::TraitT traverseTo2_, Traverse::TraitT traverseTo1_, float cost_, ConnectionID ownId_);
    const NodeID m_nodes[2];
    const Traverse::TraitT m_traverses[2];
    const float m_cost = 0.0f;
    const ConnectionID m_ownId;

    // Is on both of these nodes?
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
    NavGraph();

    NodeID makeNode(const Vector2<float> &pos_);
    ConnectionID makeConnection(NodeID node1_, NodeID node2_, Traverse::TraitT traverseTo2_, Traverse::TraitT traverseTo1_);
    std::pair<const Connection *, float> findClosestConnection(const Vector2<float> &pos_, Traverse::TraitT options_) const;

    void draw(Camera &cam_);
    Vector2<float> getConnectionCenter(const Connection &con_) const;
    float getDistToConnection(const Connection &con_, const Vector2<float> &pos_) const;

    const Connection &getConnection(ConnectionID con_) const;
    Vector2<float> getNodePos(NodeID nd_) const;

private:
    std::vector<Node> m_nodes;
    std::vector<Connection> m_connections;

    Renderer &m_ren;
    TextManager &m_textman;

    friend class NavPath;

};

#endif

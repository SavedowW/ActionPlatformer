#pragma once
#include "Core/Vector2.hpp"
#include <unordered_map>
#include <vector>

struct RoutingPoint
{
    Vector2<int> m_pos;
    int m_id;
};

struct RoutingLink
{
    RoutingPoint m_target;

    // TODO: calculate from map (speed, raw frames, etc)
    uint32_t m_duration = 60;
};

struct ColliderPointRouting
{
    RoutingPoint m_origin;
    std::vector<RoutingLink> m_links;

    // TODO: remove for release build (debug / release switch?)
    size_t m_dbgIter = 0;
};


// Shouldn't change size once level is built
using ColliderRoutesCollection = std::unordered_map<int, ColliderPointRouting>;


struct ColliderRoutingIterator
{
    ColliderRoutingIterator(const ColliderPointRouting &route_);

    const ColliderPointRouting &m_route;
    size_t m_iter = 0;
};

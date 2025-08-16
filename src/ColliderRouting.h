#ifndef COLLIDER_ROUTING_H_
#define COLLIDER_ROUTING_H_
#include "Vector2.hpp"
#include <vector>

struct RoutingPoint
{
    Vector2<float> m_pos;
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

struct ColliderRoutesCollection
{
    // Shouldn't change size once level is built
    std::map<int, ColliderPointRouting> m_routes;
};

struct ColliderRoutingIterator
{
    ColliderRoutingIterator(ColliderPointRouting &route_);

    ColliderPointRouting &m_route;
    size_t m_iter = 0;
};

#endif

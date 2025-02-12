#ifndef COLLIDER_ROUTING_H_
#define COLLIDER_ROUTING_H_
#include "Vector2.h"
#include "FrameTimer.h"
#include <vector>

struct RoutingPoint
{
    Vector2<float> m_pos;
    int m_id;
};

struct RoutingLink
{
    RoutingPoint m_target;
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
    std::map<int, ColliderPointRouting> routes;
};

#endif

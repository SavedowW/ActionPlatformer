#ifndef RESET_HANDLERS_H_
#define RESET_HANDLERS_H_
#include "CoreComponents.h"
#include "ColliderRouting.h"

template <typename... T>
class ComponentReset
{
public:
    static void resetComponent(const entt::entity &ent_, T&... comp_)
    {
        throw std::string("Component reset for type \"") + typeid(T).name() + "\" wasn't implemented";
    }
};

template <>
class ComponentReset<MoveCollider2Points, ColliderRoutingIterator>
{
public:
    static void resetComponent(const entt::entity &ent_, MoveCollider2Points &m2p_, ColliderRoutingIterator &iter_)
    {
        m2p_.m_point1 = Vector2<float>{};
        m2p_.m_point2 = Vector2<float>{};
        m2p_.m_timer.begin(0);
        
        iter_.m_iter = 0;
    }
};

#endif

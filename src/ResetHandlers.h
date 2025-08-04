#ifndef RESET_HANDLERS_H_
#define RESET_HANDLERS_H_
#include "CoreComponents.h"
#include "ColliderRouting.h"
#include "StateMachine.h"

template <typename... T>
class ComponentResetStatic
{
public:
    static void resetComponent(const entt::entity &ent_, T&... comp_);
};

template <typename... T>
class ComponentReset
{
public:
    void resetComponent(const entt::entity &ent_, T&... comp_);
};

template <>
class ComponentResetStatic<MoveCollider2Points>
{
public:
    static void resetComponent(const entt::entity &ent_, MoveCollider2Points &m2p_)
    {
        m2p_.m_point1 = Vector2<float>{};
        m2p_.m_point2 = Vector2<float>{};
        m2p_.m_timer.begin(0);
    }
};

template <>
class ComponentResetStatic<ColliderRoutingIterator>
{
public:
    static void resetComponent(const entt::entity &ent_, ColliderRoutingIterator &iter_)
    {
        iter_.m_iter = 0;
    }
};

template <>
class ComponentReset<ComponentTransform>
{
public:
    void resetComponent(const entt::entity &ent_, ComponentTransform &trans_)
    {
        trans_.m_pos = m_defaultPos;
        trans_.m_orientation = m_defaultOrientation;
    }

    Vector2<float> m_defaultPos;
    ORIENTATION m_defaultOrientation;
};

template <>
class ComponentReset<StateMachine>
{
public:
    void resetComponent(EntityAnywhere owner_, StateMachine &sm_)
    {
        sm_.switchCurrentState(owner_, m_defaultStates.begin(), m_defaultStates.end());
    }

    std::vector<CharState> m_defaultStates;
};

template <>
class ComponentResetStatic<ComponentPhysical>
{
public:
    static void resetComponent(const entt::entity &ent_, ComponentPhysical &phys_)
    {
        phys_.m_velocity = {0.0f, 0.0f};
        phys_.m_inertia = {0.0f, 0.0f};
        phys_.m_extraoffset = {0.0f, 0.0f};
        phys_.m_velocityLeftover = {0.0f, 0.0f};
        phys_.m_hitstopLeft = 0;
    }
};

template <>
class ComponentResetStatic<ComponentDynamicCameraTarget>
{
public:
    static void resetComponent(const entt::entity &ent_, ComponentDynamicCameraTarget &camtar_)
    {
        camtar_.m_offset = {0, 0};
    }
};

template <>
class ComponentResetStatic<ComponentAnimationRenderable>
{
public:
    static void resetComponent(const entt::entity &ent_, ComponentAnimationRenderable &anim_)
    {
        anim_.m_flash = nullptr;
    }
};

#endif

#pragma once
#include "ComponentsView.h"

template<typename... ComponentsT>
ComponentsView<ComponentsT...>::ComponentsView(const std::tuple<entt::entity, ComponentsT&...> &components_) :
    m_components{components_}
{}

template<typename... ComponentsT>
auto ComponentsView<ComponentsT...>::makeView(entt::registry &reg_)
{
    return reg_.view<ComponentsT...>();
}

template<typename... ComponentsT>
auto ComponentsView<ComponentsT...>::makeRefs(entt::registry &reg_, entt::entity &idx_)
{
    return std::tuple_cat(std::tuple<entt::entity>(idx_), reg_.get<ComponentsT...>(idx_));
}

template<typename... ComponentsT>
template<typename T>
T & ComponentsView<ComponentsT...>::get() const noexcept
{
    return std::get<T&>(m_components);
}

template<typename... ComponentsT>
template<typename T>
const T &ComponentsView<ComponentsT...>::cget() const noexcept
{
    return std::get<T&>(m_components);
}

template<typename... ComponentsT>
entt::entity ComponentsView<ComponentsT...>::entity() const noexcept
{
    return std::get<entt::entity>(m_components);
}

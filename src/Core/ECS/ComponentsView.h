#pragma once
#include <entt/entt.hpp>

template<typename... ComponentsT>
class ComponentsView
{
public:
    ComponentsView(const std::tuple<entt::entity, ComponentsT&...> &components_);
    
    static auto makeView(entt::registry &reg_);
    static auto makeRefs(entt::registry &reg_, entt::entity &idx_);
    

    template<typename T>
    T &get() const noexcept;

    template<typename T>
    const T &cget() const noexcept;

private:
    const std::tuple<entt::entity, ComponentsT&...> &m_components;
};

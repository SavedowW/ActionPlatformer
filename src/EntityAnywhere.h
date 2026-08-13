#pragma once
#include <entt/entt.hpp>

struct EntityAnywhere
{
    entt::registry *reg = nullptr;
    entt::entity idx = entt::null;
};

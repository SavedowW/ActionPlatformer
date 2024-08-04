#ifndef INPUT_HANDLING_SYSTEM_H_
#define INPUT_HANDLING_SYSTEM_H_
#include <entt/entt.hpp>

struct InputHandlingSystem
{
    InputHandlingSystem(entt::registry &reg_);

    void update();

    entt::registry &m_reg;
};

#endif

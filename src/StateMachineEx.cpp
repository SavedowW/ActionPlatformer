#include "StateMachineEx.hpp"
#include "Core/CoreComponents.h"
#include "Core/Vector2.hpp"

T_NAME_AUTO(UpdatePos);
T_NAME_AUTO(ChangeAnim);
T_NAME_AUTO(UpdateVelocity);
TT_NAME_AUTO(CompoundState);

void UpdatePos::update(ComponentTransform &trans_) const
{
    std::cout << FUNCNAME << std::endl;

    trans_.m_pos = {x, y};
}

void UpdateVelocity::update(ComponentPhysical &phys_, ComponentTransform &trans_) const
{
    std::cout << FUNCNAME << std::endl;

    phys_.m_velocity = {x, y};
    trans_.m_orientation = (x > 0 ? ORIENTATION::RIGHT : ORIENTATION::LEFT);
}

void ChangeAnim::update(const ComponentTransform&, const ComponentPhysical&, ComponentAnimationRenderable &ren_) const
{
    std::cout << FUNCNAME << std::endl;

    ren_.m_currentAnimation = reinterpret_cast<Animation*>(anim);
}

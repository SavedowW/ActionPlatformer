#include "StateMachineEx.hpp"

T_NAME_AUTO(UpdatePos);
T_NAME_AUTO(ChangeAnim);
T_NAME_AUTO(UpdateVelocity);
TT_NAME_AUTO(CompoundState);

void UpdatePos::update(ComponentTransform&) const
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

void UpdateVelocity::update(ComponentPhysical&, ComponentTransform&) const
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

void ChangeAnim::update(ComponentTransform&, ComponentPhysical&, ComponentAnimationRenderable&) const
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

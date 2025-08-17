#include "PlayableCharacter.hpp"

void PlayerActionWallCling::leave(EntityAnywhere owner_, CharState to_)
{
    PlayerState::leave(owner_, to_);

    if (to_ != static_cast<CharState>(CharacterState::WALL_CLING_PREJUMP))
        owner_.reg->get<ComponentPhysical>(owner_.idx).m_onWall = entt::null;

    std::cout << "Leaving wallcling" << std::endl;
}

void PlayerActionWallPrejump::leave(EntityAnywhere owner_, CharState)
{
    auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
    physical.m_onWall = entt::null;
}

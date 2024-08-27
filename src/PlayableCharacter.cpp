#include "PlayableCharacter.h"

void PlayerActionWallCling::leave(EntityAnywhere owner_, CharState to_)
{
    PlayerState::leave(owner_, to_);
    owner_.reg->get<ComponentPhysical>(owner_.idx).m_isAttached = false;
}

#pragma once
#include "SM/StateMachine.h"
#include "PlayableCharacter.h"
#include "Core/AnimationManager.h"
#include <entt/entt.hpp>

struct PlayerSystem
{
    PlayerSystem(entt::registry &reg_);

    void setup(entt::entity playerId_);
    void update();

    entt::registry &m_reg;
    AnimationManager &m_animManager;
    SM::StateMachine<PlayerState, PlayerView> m_statemachine;
};

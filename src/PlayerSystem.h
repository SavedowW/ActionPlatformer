#pragma once
#include "SM/StateMachine.h"
#include "PlayableCharacter.h"
#include "Core/AnimationManager.h"
#include <entt/entt.hpp>

class PlayerSystem
{
public:
    PlayerSystem(entt::registry &reg_);

    void createPlayer();
    void update();

    entt::entity getPlayerId() const noexcept;

private:
    entt::entity m_playerId = entt::null;
    entt::registry &m_reg;
    
    AnimationManager &m_animManager;
    SM::StateMachine<PlayerState, PlayerView> m_statemachine;
};

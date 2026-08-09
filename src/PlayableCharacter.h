#pragma once
#include "Core/Logger.h"
#include "SM/Builder.h"
#include "SM/StateProperties.h"
#include "SM/TransitionChecks.h"
#include "Core/ECS/ComponentsView.hpp"
#include "Core/CoreComponents.h"
#include "Core/StaticMapping.hpp"

enum class PlayerState : SM::StateID {
    FLOAT,
    LANDING_RECOVERY,
    IDLE,
    PRERUN,
    RUN,
    RUN_RECOVERY,
    PREJUMP,
    PREJUMP_FORWARD,
    WALL_CLING,
    WALL_CLING_PREJUMP,
    ATTACK_1,
    ATTACK_1_CHAIN,
    /*
    HARD_LANDING_RECOVERY,
    AIR_ATTACK*/
};

SERIALIZE_ENUM(PlayerState, {
    ENUM_AUTO(PlayerState, FLOAT),
    ENUM_AUTO(PlayerState, LANDING_RECOVERY),
    ENUM_AUTO(PlayerState, IDLE),
    ENUM_AUTO(PlayerState, PRERUN),
    ENUM_AUTO(PlayerState, RUN),
    ENUM_AUTO(PlayerState, RUN_RECOVERY),
    ENUM_AUTO(PlayerState, PREJUMP),
    ENUM_AUTO(PlayerState, PREJUMP_FORWARD),
    ENUM_AUTO(PlayerState, WALL_CLING),
    ENUM_AUTO(PlayerState, WALL_CLING_PREJUMP),
    ENUM_AUTO(PlayerState, ATTACK_1),
    ENUM_AUTO(PlayerState, ATTACK_1_CHAIN)
})

using PlayerView = ComponentsView<ComponentName, SM::StatePossessor<PlayerState>, ComponentTransform, ComponentPhysical,
    ComponentObstacleFallthrough, WorldPosition, ComponentAnimationRenderable, InputResolver, ComponentDynamicCameraTarget,
    ComponentChildParticles>;
using PlayerMake = SM::Make<PlayerState, PlayerView>;
using PlayerStateProperties = StateProperties<PlayerState, PlayerView>;
using PlayerStateTransitions = TransitionChecks<PlayerState, PlayerView>;

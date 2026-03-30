#pragma once
#include "SM/StateMachine.h"
#include "Core/StaticMapping.hpp"

enum class Enemy1State : SM::StateID
{
    META_ROAM,
    META_BLIND_CHASE,
    META_PROXY_SWITCH,
    META_MOVE_TOWARDS,
    META_NAVIGATE_GRAPH_CHASE,
    IDLE,
    FLOAT,
    PREJUMP,
    RUN
};

SERIALIZE_ENUM(Enemy1State, {
    ENUM_AUTO(Enemy1State, META_ROAM),
    ENUM_AUTO(Enemy1State, META_BLIND_CHASE),
    ENUM_AUTO(Enemy1State, META_PROXY_SWITCH),
    ENUM_AUTO(Enemy1State, META_MOVE_TOWARDS),
    ENUM_AUTO(Enemy1State, META_NAVIGATE_GRAPH_CHASE),
    ENUM_AUTO(Enemy1State, IDLE),
    ENUM_AUTO(Enemy1State, FLOAT),
    ENUM_AUTO(Enemy1State, PREJUMP),
    ENUM_AUTO(Enemy1State, RUN)
})

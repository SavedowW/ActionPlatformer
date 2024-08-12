#ifndef ENEMY_1_H_
#define ENEMY_1_H_
#include "CoreComponents.h"
#include "StateMachine.h"
#include <map>

enum class Enemy1State : CharState
{
    META_ROAM,
    IDLE,
    FLOAT,
    RUN,
    NONE
};

inline const std::map<Enemy1State, std::string> Enemy1StateNames {
    {Enemy1State::META_ROAM, "META_ROAM"},
    {Enemy1State::IDLE, "IDLE"},
    {Enemy1State::FLOAT, "FLOAT"},
    {Enemy1State::RUN, "RUN"},
};

#endif
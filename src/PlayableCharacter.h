#ifndef PLAYABLE_CHARACTER_H_
#define PLAYABLE_CHARACTER_H_
#include "CoreComponents.h"
#include "StateMachine.hpp"
#include <map>
#include <string>

enum class CharacterState {
    IDLE,
    RUN
};

inline const std::map<CharacterState, std::string> CharacterStateNames {
    {CharacterState::IDLE, "IDLE"},
    {CharacterState::RUN, "RUN"}
};

#endif

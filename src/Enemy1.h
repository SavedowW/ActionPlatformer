#ifndef ENEMY_1_H_
#define ENEMY_1_H_
#include "CoreComponents.h"
#include "StateMachine.h"
#include "CommonAI.h"
#include <map>

enum class Enemy1State : CharState
{
    META_ROAM,
    META_BLIND_CHASE,
    META_PROXY_SWITCH,
    IDLE,
    FLOAT,
    RUN,
    NONE
};

inline const std::map<Enemy1State, std::string> Enemy1StateNames {
    {Enemy1State::META_ROAM, "META_ROAM"},
    {Enemy1State::META_BLIND_CHASE, "META_BLIND_CHASE"},
    {Enemy1State::META_PROXY_SWITCH, "META_PROXY_SWITCH"},
    {Enemy1State::IDLE, "IDLE"},
    {Enemy1State::FLOAT, "FLOAT"},
    {Enemy1State::RUN, "RUN"},
};

template<bool REQUESTED_ONLY, bool HOLD_WHILE_REQUESTED>
class NPCState : public PhysicalState
{
public:
    template<typename PLAYER_STATE_T>
    NPCState(PLAYER_STATE_T stateId_, const std::string &stateName_, StateMarker &&transitionableFrom_, int anim_) :
        PhysicalState(stateId_, stateName_, std::move(transitionableFrom_), anim_)
    {
    }

    inline virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_)
    {
        auto res = PhysicalState::update(owner_, currentFrame_);

        if constexpr (HOLD_WHILE_REQUESTED)
        {
            auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
            auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);

            if (ai.m_requestedState.has_value() && m_stateId == ai.m_requestedState && 
                (ai.m_requestedOrientation == ORIENTATION::UNSPECIFIED || ai.m_requestedOrientation == trans.m_orientation))
                return false;
        }
        else
            return true;

        return true;
    }

    inline virtual ORIENTATION isPossible(EntityAnywhere owner_) const override
    {
        if (PhysicalState::isPossible(owner_) == ORIENTATION::UNSPECIFIED)
            return ORIENTATION::UNSPECIFIED;

        auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
        auto curstate = m_parent->m_currentState->m_stateId;

        if constexpr (REQUESTED_ONLY)
        {
            if (!ai.m_requestedState.has_value() || ai.m_requestedState != m_stateId)
                return ORIENTATION::UNSPECIFIED;
        }

        if (m_stateId == curstate)
        {
            if (ai.m_requestedOrientation != owner_.reg->get<ComponentTransform>(owner_.idx).m_orientation)
                return ai.m_requestedOrientation;
            else
                return ORIENTATION::UNSPECIFIED;
        }

        if (ai.m_requestedOrientation != ORIENTATION::UNSPECIFIED)
            return ai.m_requestedOrientation;
        else
            return owner_.reg->get<ComponentTransform>(owner_.idx).m_orientation;
    }

protected:


};

#endif
#ifndef ENEMY_1_H_
#define ENEMY_1_H_
#include "CoreComponents.h"
#include "StateMachine.h"
#include "CommonAI.h"
#include "EnumMapping.hpp"
#include <map>

enum class Enemy1State : CharState
{
    META_ROAM,
    META_BLIND_CHASE,
    META_PROXY_SWITCH,
    META_MOVE_TOWARDS,
    META_NAVIGATE_GRAPH_CHASE,
    IDLE,
    FLOAT,
    PREJUMP,
    RUN,
    NONE
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

template<bool REQUESTED_ONLY, bool HOLD_WHILE_REQUESTED>
class NPCState : public PhysicalState
{
public:
    template<typename PLAYER_STATE_T>
    NPCState(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_, ResID anim_) :
        PhysicalState(stateId_, std::move(transitionableFrom_), anim_)
    {
    }

    inline virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_)
    {
        auto res = PhysicalState::update(owner_, currentFrame_);

        if constexpr (HOLD_WHILE_REQUESTED)
        {
            auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
            const auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);

            if (ai.m_requestedState.has_value() && m_stateId == ai.m_requestedState && 
                (ai.m_requestedOrientation == ORIENTATION::UNSPECIFIED || ai.m_requestedOrientation == trans.m_orientation))
                return false;
            else
                return res;
        }
        else
            return res;
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

class AimedPrejump : public NPCState<true, false>
{
public:
    template<typename PLAYER_STATE_T>
    AimedPrejump(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_, ResID anim_, float gravity_, float maxInitialHorSpd_) :
        NPCState<true, false>(stateId_, std::move(transitionableFrom_), anim_),
        m_gravity(gravity_),
        m_maxInitialHorSpd(maxInitialHorSpd_)
    {}

    virtual void onOutdated(EntityAnywhere owner_) override;

private:
    float m_gravity = 0.0f;
    float m_maxInitialHorSpd = 0.0f;

};

class AimedFloat : public NPCState<false, false>
{
public:
    template<typename PLAYER_STATE_T>
    AimedFloat(PLAYER_STATE_T stateId_, StateMarker &&transitionableFrom_, ResID anim_) :
        NPCState<false, false>(stateId_, std::move(transitionableFrom_), anim_)
    {}

    virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override;
    virtual void leave(EntityAnywhere owner_, CharState to_) override;
};

#endif
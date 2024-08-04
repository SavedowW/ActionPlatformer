#ifndef PLAYABLE_CHARACTER_H_
#define PLAYABLE_CHARACTER_H_
#include "CoreComponents.h"
#include "StateMachine.h"
#include "InputComparators.h"
#include <map>
#include <string>

enum class CharacterState {
    IDLE,
    FLOAT,
    RUN,
    PREJUMP,
    PREJUMP_FORWARD,
    WALL_CLING,
    WALL_CLING_PREJUMP,
    NONE
};

inline const std::map<CharacterState, std::string> CharacterStateNames {
    {CharacterState::IDLE, "IDLE"},
    {CharacterState::FLOAT, "IDLE"},
    {CharacterState::RUN, "RUN"},
    {CharacterState::PREJUMP, "PREJUMP"},
    {CharacterState::PREJUMP_FORWARD, "PREJUMP_FORWARD"},
    {CharacterState::WALL_CLING, "WALL_CLING"},
    {CharacterState::WALL_CLING_PREJUMP, "WALL_CLING_PREJUMP"}
};

template<bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN>
inline ORIENTATION attemptInput(const InputComparator &cmpL_, const InputComparator &cmpR_, ORIENTATION currentOrientation_, const InputQueue &iq_, int val_)
{
    if (REQUIRE_ALIGNMENT)
    {
        if (currentOrientation_ == ORIENTATION::RIGHT && cmpR_(iq_, val_) || currentOrientation_ == ORIENTATION::LEFT && cmpL_(iq_, val_))
            return currentOrientation_;
    }
    else
    {
        auto lValid = cmpL_(iq_, val_);
        auto rValid = cmpR_(iq_, val_);

        if (rValid && currentOrientation_ == ORIENTATION::RIGHT)
            return ORIENTATION::RIGHT;

        if (lValid && currentOrientation_ == ORIENTATION::LEFT)
            return ORIENTATION::LEFT;

        if (!FORCE_REALIGN)
            return ORIENTATION::UNSPECIFIED;

        if (rValid)
            return ORIENTATION::RIGHT;
        else if (lValid)
            return ORIENTATION::LEFT;
    }

    return ORIENTATION::UNSPECIFIED;
}

template<bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN,
    typename CMP_LEFT, typename CMP_RIGHT,
    bool ATTEMPT_PROCEED, typename CMP_PROCEED_LEFT, typename CMP_PROCEED_RIGHT>
class PlayerState : public GenericState
{
public:
    PlayerState(CharacterState actionState_, StateMarker &&transitionableFrom_, int anim_) :
        GenericState(actionState_, CharacterStateNames.at(actionState_), std::move(transitionableFrom_), anim_)
    {
    }

    inline virtual void enter(EntityAnywhere owner_, CharState from_) override
    {
        GenericState::enter(owner_, from_);

        /* TODO: realign for input direction

        auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        if (m_realignOnSwitchForInput)
        {
            auto indir = m_inputResolver.getCurrentInputDir();
            if (indir.x > 0)
                ParentClass::m_transform.m_ownOrientation = ORIENTATION::RIGHT;
            else if (indir.x < 0)
                ParentClass::m_transform.m_ownOrientation = ORIENTATION::LEFT;
        }
        */
    }

    inline virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_)
    {
        auto res = GenericState::update(owner_, currentFrame_);

        auto &compInput = owner_.reg->get<ComponentPlayerInput>(owner_.idx);
        auto &compFallthrough = owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx);
        if (compInput.m_inputResolver->getInputQueue()[0].m_inputs.at(INPUT_BUTTON::DOWN) == INPUT_BUTTON_STATE::PRESSED)
            compFallthrough.setIgnoringObstacles();

        if (!res)
            return false;

        if (!ATTEMPT_PROCEED)
            return true;

        auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);

        auto orientation = transform.m_orientation;
        const auto &inq = compInput.m_inputResolver->getInputQueue();

        bool possibleToLeft = (!m_alignedSlopeMax.isSet() || physical.m_onSlopeWithAngle <= 0 || physical.m_onSlopeWithAngle <= m_alignedSlopeMax);
        bool possibleToRight = (!m_alignedSlopeMax.isSet() || physical.m_onSlopeWithAngle >= 0 || -physical.m_onSlopeWithAngle <= m_alignedSlopeMax);

        InputComparatorFail failin;

        auto &lInput = (possibleToLeft ? static_cast<const InputComparator&>(m_cmpProcLeft) : static_cast<const InputComparator&>(failin));
        auto &rInput = (possibleToRight ? static_cast<const InputComparator&>(m_cmpProcRight) : static_cast<const InputComparator&>(failin));

        auto inres = attemptInput<true, false>(lInput, rInput, orientation, inq, 0);
        return inres == ORIENTATION::UNSPECIFIED;

    }

    inline virtual ORIENTATION isPossible(EntityAnywhere owner_) const override
    {
        if (GenericState::isPossible(owner_) == ORIENTATION::UNSPECIFIED)
            return ORIENTATION::UNSPECIFIED;

        auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &compInput = owner_.reg->get<ComponentPlayerInput>(owner_.idx);

        auto orientation = transform.m_orientation;
        const auto &inq = compInput.m_inputResolver->getInputQueue();
        auto currentState = GenericState::m_parent->m_currentState;

        bool possibleToLeft = (!m_alignedSlopeMax.isSet() || physical.m_onSlopeWithAngle <= 0 || physical.m_onSlopeWithAngle <= m_alignedSlopeMax);
        bool possibleToRight = (!m_alignedSlopeMax.isSet() || physical.m_onSlopeWithAngle >= 0 || -physical.m_onSlopeWithAngle <= m_alignedSlopeMax);

        InputComparatorFail failin;

        auto &lInput = (possibleToLeft ? static_cast<const InputComparator&>(m_cmpLeft) : static_cast<const InputComparator&>(failin));
        auto &rInput = (possibleToRight ? static_cast<const InputComparator&>(m_cmpRight) : static_cast<const InputComparator&>(failin));

        return attemptInput<REQUIRE_ALIGNMENT, FORCE_REALIGN>(lInput, rInput, orientation, inq, 0);
    }

    inline PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT> 
        &setAlignedSlopeMax(float alignedSlopeMax_)
    {
        m_alignedSlopeMax = alignedSlopeMax_;
        return *this;
    }

    inline PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT> 
        &setRealignOnSwitch(bool realignOnSwitch_)
    {
        m_realignOnSwitchForInput = realignOnSwitch_;
        return *this;
    }

protected:
    const Collider m_hurtbox;
    CMP_LEFT m_cmpLeft;
    CMP_RIGHT m_cmpRight;
    CMP_PROCEED_LEFT m_cmpProcLeft;
    CMP_PROCEED_RIGHT m_cmpProcRight;

    utils::OptionalProperty<float> m_alignedSlopeMax;
    bool m_realignOnSwitchForInput = false;
};


class PlayerActionFloat: public PlayerState<false, true, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>
{
public:
    PlayerActionFloat(int anim_, StateMarker transitionableFrom_) :
        PlayerState<false, true, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>(CharacterState::FLOAT, std::move(transitionableFrom_), anim_)
    {
    }

    inline virtual void enter(EntityAnywhere owner_, CharState from_) override
    {
        ParentAction::enter(owner_, from_);

        auto &fallthrough = owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx);
        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        if (fallthrough.isIgnoringAllObstacles() && abs(phys.m_velocity.x) > 0.8f)
            phys.m_velocity.y += 5.0f;
    }

    inline virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override
    {
        auto res = ParentAction::update(owner_, currentFrame_);

        const auto &inq = owner_.reg->get<ComponentPlayerInput>(owner_.idx).m_inputResolver->getInputQueue();
        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);

        if (m_driftLeftInput(inq, 0))
        {
            if (phys.m_velocity.x > -4.0f)
                phys.m_velocity.x -= 0.15f;
        }

        if (m_driftRightInput(inq, 0))
        {
            if (phys.m_velocity.x < 4.0f)
                phys.m_velocity.x += 0.15f;
        }

        if (phys.m_velocity.y < 0 && m_driftUpInput(inq, 0))
        {
            if (m_parent->m_framesInState < 10.0f)
                phys.m_velocity.y -= 0.4f;
        }

        auto total = phys.m_velocity.x + phys.m_inertia.x;
        if (total > 0)
            trans.m_orientation = ORIENTATION::RIGHT;
        else if (total < 0)
            trans.m_orientation = ORIENTATION::LEFT;

        if (phys.m_velocity.y + phys.m_inertia.y > 0)
            phys.m_gravity *= 1.3f;

        return res;
    }

protected:
    using ParentAction = PlayerState<false, true, InputComparatorIdle, InputComparatorIdle, false, InputComparatorIdle, InputComparatorIdle>;
    InputComparatorHoldLeft m_driftLeftInput;
    InputComparatorHoldRight m_driftRightInput;
    InputComparatorHoldUp m_driftUpInput;

};

class PlayerActionWallCling: public PlayerState<false, true, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail>
{
public:
    PlayerActionWallCling(int anim_, StateMarker transitionableFrom_) :
        PlayerState<false, true, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail>(CharacterState::WALL_CLING, std::move(transitionableFrom_), anim_),
        m_transitionOnLeave(CharacterState::FLOAT)
    {
        setGravity(TimelineProperty<Vector2<float>>({0.0f, 0.020f}));
        setConvertVelocityOnSwitch(true);
    }

    inline virtual void enter(EntityAnywhere owner_, CharState from_) override
    {
        ParentAction::enter(owner_, from_);

        auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);

        if (physical.m_inertia.y > 0)
            physical.m_inertia.y = 0;
        if (physical.m_velocity.y > 0)
            physical.m_velocity.y = 0;

        physical.m_velocity.x = 0;
        physical.m_inertia.x = 0;
    }

    inline virtual bool update(EntityAnywhere owner_, uint32_t currentFrame_) override
    {
        auto res = ParentAction::update(owner_, currentFrame_);

        auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &compInput = owner_.reg->get<ComponentPlayerInput>(owner_.idx);
        auto &cworld = owner_.reg->get<World>(owner_.idx);
        auto pb = physical.m_pushbox + transform.m_pos;

        auto orientation = transform.m_orientation;

        bool stillValid = (orientation == ORIENTATION::RIGHT ? 
            cworld.touchingWallAt(ORIENTATION::RIGHT, pb.m_center - Vector2{pb.m_halfSize.x, 0.0f}) :
            cworld.touchingWallAt(ORIENTATION::LEFT, pb.m_center + Vector2{pb.m_halfSize.x, 0.0f}));

        if (!stillValid)
        {
            if (physical.getPosOffest().y < 0)
                physical.m_velocity.y -= 5.0f;
            
            m_parent->switchCurrentState(owner_, m_transitionOnLeave);
        }

        return false;
    }

    inline virtual ORIENTATION isPossible(EntityAnywhere owner_) const override
    {
        if (GenericState::isPossible(owner_) == ORIENTATION::UNSPECIFIED)
            return ORIENTATION::UNSPECIFIED;

        auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
        auto &compInput = owner_.reg->get<ComponentPlayerInput>(owner_.idx);
        auto &cworld = owner_.reg->get<World>(owner_.idx);
        auto &cobs = owner_.reg->get<ComponentObstacleFallthrough>(owner_.idx);

        auto orientation = transform.m_orientation;
        const auto &inq = compInput.m_inputResolver->getInputQueue();
        auto currentState = GenericState::m_parent->m_currentState;
        auto pb = physical.m_pushbox + transform.m_pos;

        if (cworld.touchingGround(pb, cobs))
            return ORIENTATION::UNSPECIFIED;

        if (orientation == ORIENTATION::RIGHT)
        {
            if (m_cmpLeft(inq, 0))
            {
                if (cworld.touchingWallAt(ORIENTATION::LEFT, pb.m_center + Vector2{pb.m_halfSize.x, 0.0f})) 
                    return ORIENTATION::LEFT;
            }
        }
        else if (orientation == ORIENTATION::LEFT)
        {
            if (m_cmpRight(inq, 0))
            {
                if (cworld.touchingWallAt(ORIENTATION::RIGHT, pb.m_center - Vector2{pb.m_halfSize.x, 0.0f})) 
                    return ORIENTATION::RIGHT;
            }
        }

        return ORIENTATION::UNSPECIFIED;
    }

protected:
    using ParentAction = PlayerState<false, true, InputComparatorBufferedHoldRight, InputComparatorBufferedHoldLeft, false, InputComparatorFail, InputComparatorFail>;
    CharacterState m_transitionOnLeave;
};

#endif

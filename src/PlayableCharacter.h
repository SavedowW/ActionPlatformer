#ifndef PLAYABLE_CHARACTER_H_
#define PLAYABLE_CHARACTER_H_
#include "CoreComponents.h"
#include "StateMachine.hpp"
#include "yaECS.hpp"
#include "InputComparators.h"
#include <map>
#include <string>

using ArchPlayer = ECS::EntityData<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, ComponentAnimationRenderable, ComponentPlayerInput>;

enum class CharacterState {
    IDLE,
    FLOAT,
    RUN,
    PREJUMP,
    PREJUMP_FORWARD,
    NONE
};

inline const std::map<CharacterState, std::string> CharacterStateNames {
    {CharacterState::IDLE, "IDLE"},
    {CharacterState::FLOAT, "IDLE"},
    {CharacterState::RUN, "RUN"},
    {CharacterState::PREJUMP, "PREJUMP"},
    {CharacterState::PREJUMP_FORWARD, "PREJUMP_FORWARD"}
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
class PlayerState : public GenericState<ArchPlayer::MakeRef, CharacterState>
{
public:
    PlayerState(CharacterState actionState_, StateMarker<CharacterState> &&transitionableFrom_, int anim_) :
        GenericState<ArchPlayer::MakeRef, CharacterState>(actionState_, CharacterStateNames.at(actionState_), std::move(transitionableFrom_), anim_)
    {
    }

    inline virtual void enter(ArchPlayer::MakeRef &owner_, CharacterState from_) override
    {
        ParentClass::enter(owner_, from_);

        auto &transform = std::get<ComponentTransform&>(owner_);

        /* TODO: realign for input direction
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

    inline virtual bool update(ArchPlayer::MakeRef &owner_, uint32_t currentFrame_)
    {
        auto res = ParentClass::update(owner_, currentFrame_);

        auto &compInput = std::get<ComponentPlayerInput&>(owner_);
        auto &compFallthrough = std::get<ComponentObstacleFallthrough&>(owner_);
        if (compInput.m_inputResolver->getInputQueue()[0].m_inputs.at(INPUT_BUTTON::DOWN) == INPUT_BUTTON_STATE::PRESSED)
            compFallthrough.setIgnoringObstacles();

        if (!res)
            return false;

        if (!ATTEMPT_PROCEED)
            return true;

        auto &transform = std::get<ComponentTransform&>(owner_);
        auto &physical = std::get<ComponentPhysical&>(owner_);

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

    inline virtual ORIENTATION isPossible(ArchPlayer::MakeRef &owner_) const override
    {
        if (ParentClass::isPossible(owner_) == ORIENTATION::UNSPECIFIED)
            return ORIENTATION::UNSPECIFIED;

        auto &transform = std::get<ComponentTransform&>(owner_);
        auto &physical = std::get<ComponentPhysical&>(owner_);
        auto &compInput = std::get<ComponentPlayerInput&>(owner_);

        auto orientation = transform.m_orientation;
        const auto &inq = compInput.m_inputResolver->getInputQueue();
        auto currentState = ParentClass::m_parent->m_currentState;

        ORIENTATION SlopeDir = ORIENTATION::UNSPECIFIED;
        if (physical.m_onSlopeWithAngle > 0)
            SlopeDir = ORIENTATION::RIGHT;
        else if (physical.m_onSlopeWithAngle < 0)
            SlopeDir = ORIENTATION::LEFT;

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
    using ParentClass = GenericState<ArchPlayer::MakeRef, CharacterState>;
    const Collider m_hurtbox;
    CMP_LEFT m_cmpLeft;
    CMP_RIGHT m_cmpRight;
    CMP_PROCEED_LEFT m_cmpProcLeft;
    CMP_PROCEED_RIGHT m_cmpProcRight;

    utils::OptionalProperty<float> m_alignedSlopeMax;
    bool m_realignOnSwitchForInput = false;
};

#endif

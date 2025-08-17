#ifndef PLAYABLE_CHARACTER_HPP_
#define PLAYABLE_CHARACTER_HPP_
#include "PlayableCharacter.h"

template<bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN, bool FORCE_TOWARDS_INPUT,
    typename CMP_LEFT, typename CMP_RIGHT,
    bool ATTEMPT_PROCEED, typename CMP_PROCEED_LEFT, typename CMP_PROCEED_RIGHT>
ORIENTATION PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, FORCE_TOWARDS_INPUT,
    CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT>::isPossible(EntityAnywhere owner_) const
{
    if (PhysicalState::isPossible(owner_) == ORIENTATION::UNSPECIFIED)
        return ORIENTATION::UNSPECIFIED;

    const auto &transform = owner_.reg->get<ComponentTransform>(owner_.idx);
    const auto &physical = owner_.reg->get<ComponentPhysical>(owner_.idx);
    const auto &compInput = owner_.reg->get<InputResolver>(owner_.idx);

    const auto orientation = transform.m_orientation;
    const auto &inq = compInput.getInputQueue();

    const bool possibleToLeft = (!m_alignedSlopeMax.has_value() || physical.m_onSlopeWithAngle <= 0 || physical.m_onSlopeWithAngle <= m_alignedSlopeMax) && (!FORCE_TOWARDS_INPUT || inq[0].m_dir.x <= 0);
    const bool possibleToRight = (!m_alignedSlopeMax.has_value() || physical.m_onSlopeWithAngle >= 0 || -physical.m_onSlopeWithAngle <= m_alignedSlopeMax) && (!FORCE_TOWARDS_INPUT || inq[0].m_dir.x >= 0);

    if (possibleToLeft && possibleToRight)
        return attemptInput<REQUIRE_ALIGNMENT, FORCE_REALIGN | FORCE_TOWARDS_INPUT, CMP_LEFT, CMP_RIGHT>(orientation, inq, m_extendedBuffer);
    else if (possibleToLeft)
        return attemptInput<REQUIRE_ALIGNMENT, FORCE_REALIGN | FORCE_TOWARDS_INPUT, CMP_LEFT, InputComparatorFail >(orientation, inq, m_extendedBuffer);
    else if (possibleToRight)
        return attemptInput<REQUIRE_ALIGNMENT, FORCE_REALIGN | FORCE_TOWARDS_INPUT, InputComparatorFail, CMP_RIGHT >(orientation, inq, m_extendedBuffer);

    return ORIENTATION::UNSPECIFIED;
}

template<bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN, bool FORCE_TOWARDS_INPUT,
    typename CMP_LEFT, typename CMP_RIGHT,
    bool ATTEMPT_PROCEED, typename CMP_PROCEED_LEFT, typename CMP_PROCEED_RIGHT>
inline auto
    &PlayerState<REQUIRE_ALIGNMENT, FORCE_REALIGN, FORCE_TOWARDS_INPUT,
    CMP_LEFT, CMP_RIGHT, ATTEMPT_PROCEED, CMP_PROCEED_LEFT, CMP_PROCEED_RIGHT>::setAlignedSlopeMax(float alignedSlopeMax_)
{
    m_alignedSlopeMax = alignedSlopeMax_;
    return *this;
}

#endif

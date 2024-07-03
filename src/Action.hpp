#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "Object.h"
#include "InputResolver.h"
#include "InputComparators.h"

/*
    Attempt provided input considering alignment rules
    if REQUIRE_ALIGNMENT is true, only input in specified direction is attempted
    if FORCE_REALIGN is true and real input direction conflicts with specified, input is still valid
    Input in specified direction is ALWAYS prioritized
    currentOrientation_ is expected to be LEFT or RIGHT, not UNSPECIFIED
    Return value is valid direction of input, or UNSPECIFIED if none is valid
*/



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

class GenericAction
{
public:
    inline virtual ORIENTATION isPossibleInDirection(int extendBuffer_) const = 0;
};

template<typename CHAR_STATES, bool REQUIRE_ALIGNMENT, bool FORCE_REALIGN, typename CMP_LEFT, typename CMP_RIGHT, typename OWNER>
class Action : public GenericAction
{
public:
    Action(CHAR_STATES actionState_, const Collider &hurtbox_, int anim_, StateMarker transitionableFrom_, OWNER &owner_, const InputResolver &inputResolver_) :
        m_ownState(actionState_),
        m_hurtbox(hurtbox_),
        m_anim(anim_),
        m_transitionableFrom(std::move(transitionableFrom_)),
        m_owner(owner_),
        m_inputResolver(inputResolver_)
    {

    }

    inline virtual ORIENTATION isPossibleInDirection(int extendBuffer_) const override
    {
        return attemptInput<REQUIRE_ALIGNMENT, FORCE_REALIGN>(m_cmpLeft, m_cmpRight, m_owner.getOwnOrientation(), m_inputResolver.getInputQueue(), extendBuffer_);
    }

private:
    CHAR_STATES m_ownState;
    const Collider m_hurtbox;
    int m_anim;
    StateMarker m_transitionableFrom;
    CMP_LEFT m_cmpLeft;
    CMP_RIGHT m_cmpRight;
    OWNER &m_owner;
    const InputResolver &m_inputResolver;
};

#endif

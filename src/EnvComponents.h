#ifndef ENV_COMPONENTS_H_
#define ENV_COMPONENTS_H_
#include "World.h"
#include "Core/Collider.h"

struct GrassTopComp
{
public:
    void touchedPlayer(const Vector2<float> &velocity_, EntityAnywhere self_);

    // True if requires collision check
    bool update(EntityAnywhere self_);

    static const Collider colliderRight;
    static const Collider colliderLeft;
    static ResID m_idleAnimId;
    static ResID m_flickLeftAnimId;
    static ResID m_flickRightAnimId;

    enum class State
    {
        IDLE,
        FLICK_LEFT,
        FLICK_RIGHT
    } m_state = State::IDLE;
};

#endif

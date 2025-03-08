#ifndef ENV_COMPONENTS_H_
#define ENV_COMPONENTS_H_
#include "World.h"
#include "Collider.h"

struct GrassTopComp
{
public:
    void touchedPlayer(const Vector2<float> &velocity_, EntityAnywhere self_);

    // True if requires collision check
    bool update(EntityAnywhere self_);

    static const Collider m_colliderRight;
    static const Collider m_colliderLeft;
    static int m_idleAnimId;
    static int m_flickLeftAnimId;
    static int m_flickRightAnimId;

    enum class State
    {
        IDLE,
        FLICK_LEFT,
        FLICK_RIGHT
    } m_state = State::IDLE;
};

#endif

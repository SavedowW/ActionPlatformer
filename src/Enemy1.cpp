#include "Enemy1.h"

void AimedPrejump::onOutdated(EntityAnywhere owner_)
{
    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
    auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);

    ai.m_isNavigating = true;

    auto realspd = phys.m_velocity + phys.m_inertia;

    auto peak = 5.0f + (rand() % 10);
    auto delta = ai.m_navigationTarget - (trans.m_pos - Vector2{0.0f, 8.0f});
    delta.y -= peak;
    //delta.x += utils::signof(delta.x) * ((rand() % 10));

    auto signx = utils::signof(delta.x);

    Vector2<float> initialImpulse;
    initialImpulse.y = -sqrt(2 * m_gravity * abs(delta.y));

    auto duration = abs(initialImpulse.y / m_gravity) + sqrt(2 * peak / m_gravity);
    auto dx = abs(delta.x);
    if (duration * (m_maxInitialHorSpd + realspd.x) >= dx)
    {
        initialImpulse.x = signx * dx / duration - realspd.x;
        ai.m_additionalAccel = 0.01f;
    }
    else
    {
        auto fullrealspd = realspd.x + signx * m_maxInitialHorSpd;
        initialImpulse.x = signx * m_maxInitialHorSpd;
        ai.m_additionalAccel = 2 * (dx - fullrealspd * duration) / duration / duration;
    }

    phys.m_velocity += initialImpulse;

    std::cout << "New velocity: " << phys.m_velocity << " (" << ai.m_additionalAccel << ")" << std::endl;

    NPCState::onOutdated(owner_);
}

bool AimedFloat::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);

    if (ai.m_isNavigating)
    {
        auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        //auto pb = phys.m_pushbox + trans.m_pos;
        auto delta = ai.m_navigationTarget - trans.m_pos;
        //auto onPoint = (abs(ai.m_navigationTarget.x - pb.m_center.x) < pb.m_halfSize.x);

        if (delta.x != 0.0f)
            phys.m_velocity.x += utils::signof(delta.x) * ai.m_additionalAccel;

        std::cout << "d(" << delta << ") " << phys.m_velocity << std::endl;
    }

    return NPCState::update(owner_, currentFrame_);
}

void AimedFloat::leave(EntityAnywhere owner_, CharState to_)
{
    owner_.reg->get<ComponentAI>(owner_.idx).m_isNavigating = false;
    return NPCState::leave(owner_, to_);
}

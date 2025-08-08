#include "Enemy1.h"

void AimedPrejump::onOutdated(EntityAnywhere owner_)
{
    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    const auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
    auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);

    ai.m_isNavigating = true;

    const auto currentspd = phys.m_velocity + phys.m_inertia;

    // Max reached height over the destination
    const auto peak = 5.0f + (rand() % 10);

    /*
        Distance from the player to the jump target pos, but at the peak height
        TODO: 8 px is the usual offset for nodes from the ground, should be a var
    */
    const auto delta = (ai.m_navigationTarget + Vector2{0.0f, 8.0f} - Vector2{0.0f, peak}) - trans.m_pos;
    //delta.x += utils::signof(delta.x) * ((rand() % 10));

    const auto jumpdir = utils::signof(delta.x);

    Vector2<float> initialImpulse;

    // Calculated so that with m_gravity the velocity will turn 0 at height delta.y (from starting point)
    initialImpulse.y = -sqrt(2 * m_gravity * abs(delta.y));

    const auto duration = abs(initialImpulse.y / m_gravity) + sqrt(2 * peak / m_gravity);
    if (utils::isLowerOrGreater(duration * (m_maxInitialHorSpd + currentspd.x), delta.x))
    {
        initialImpulse.x = delta.x / duration - currentspd.x;
        ai.m_additionalAccel = 0.01f;
    }
    else
    {
        initialImpulse.x = jumpdir * m_maxInitialHorSpd;
        const auto realNewSpeed = currentspd.x + initialImpulse.x;
        ai.m_additionalAccel = 2 * (delta.x - realNewSpeed * duration) / duration / duration;
    }

    phys.m_velocity += initialImpulse;

    //std::cout << "New velocity: " << phys.m_velocity << " (" << ai.m_additionalAccel << ")" << std::endl;

    NPCState::onOutdated(owner_);
}

bool AimedFloat::update(EntityAnywhere owner_, uint32_t currentFrame_)
{
    const auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);

    if (ai.m_isNavigating)
    {
        const auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
        auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);
        //auto pb = phys.m_pushbox + trans.m_pos;
        auto delta = ai.m_navigationTarget - trans.m_pos;
        //auto onPoint = (abs(ai.m_navigationTarget.x - pb.m_center.x) < pb.m_halfSize.x);

        if (delta.x != 0.0f)
            phys.m_velocity.x += utils::signof(delta.x) * ai.m_additionalAccel;

        //std::cout << "d(" << delta << ") vel:{" << phys.m_velocity << "} inr:{" << phys.m_inertia << "} pos:{" << trans.m_pos << "}" << std::endl;
    }

    return NPCState::update(owner_, currentFrame_);
}

void AimedFloat::leave(EntityAnywhere owner_, CharState to_)
{
    owner_.reg->get<ComponentAI>(owner_.idx).m_isNavigating = false;
    return NPCState::leave(owner_, to_);
}

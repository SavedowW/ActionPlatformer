#include "Enemy1.h"

void AimedPrejump::onOutdated(EntityAnywhere owner_)
{
    auto &ai = owner_.reg->get<ComponentAI>(owner_.idx);
    auto &trans = owner_.reg->get<ComponentTransform>(owner_.idx);
    auto &phys = owner_.reg->get<ComponentPhysical>(owner_.idx);

    auto delta = ai.m_navigationTarget - trans.m_pos;

    phys.m_velocity.y += std::min((delta.y / 10.0f), 4.0f);
    phys.m_velocity.x += std::min((delta.x / 10.0f) * 3.0f, 4.0f);
    std::cout << "New velocity: " << phys.m_velocity << std::endl;

    NPCState::onOutdated(owner_);
}

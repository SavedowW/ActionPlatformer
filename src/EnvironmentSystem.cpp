#include "EnvironmentSystem.h"
#include "CoreComponents.h"
#include "EnvComponents.h"

EnvironmentSystem::EnvironmentSystem(Application &app_, entt::registry &reg_) :
    m_app(app_),
    m_reg(reg_)
{
}

void EnvironmentSystem::update(entt::entity playerId_)
{
    auto physicals = m_reg.view<ComponentTransform, ComponentPhysical>();
    auto grassTops = m_reg.view<ComponentTransform, GrassTopComp>();

    for (auto [idx, trans, grass] : grassTops.each())
    {
        if (grass.update({&m_reg, idx}))
        {
            for (auto [idx2, trans2, phys2] : physicals.each())
            {
                Collider grassPb;
                if (phys2.m_appliedOffset.x > 0.01f)
                    grassPb = GrassTopComp::m_colliderRight + trans.m_pos;
                else if (phys2.m_appliedOffset.x <= -0.01f)
                    grassPb = GrassTopComp::m_colliderLeft + trans.m_pos;
                else
                    continue;
                    
                auto pb = phys2.m_pushbox + trans2.m_pos;
                auto res = pb.checkOverlap(grassPb);
                if (checkCollision(res, OverlapResult::OVERLAP_BOTH))
                    grass.touchedPlayer(phys2.m_appliedOffset, {&m_reg, idx});
            }
        }
    }
}

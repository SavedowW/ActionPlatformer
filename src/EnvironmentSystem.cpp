#include "EnvironmentSystem.h"
#include "EnvComponents.h"
#include "Core/CoreComponents.h"

EnvironmentSystem::EnvironmentSystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void EnvironmentSystem::update()
{
    auto physicals = m_reg.view<ComponentTransform, ComponentPhysical>();
    auto grassTops = m_reg.view<ComponentTransform, GrassTopComp>();

    for (auto [idx, trans, grass] : grassTops.each())
    {
        if (grass.update({.reg=&m_reg, .idx=idx}))
        {
            for (const auto &[idx2, trans2, phys2] : physicals.each())
            {
                const auto avgAppliedOffset = phys2.appliedOffset.avg();
                Collider grassPb;
                if (avgAppliedOffset.x > 0)
                    grassPb = GrassTopComp::colliderRight + trans.m_pos;
                else if (avgAppliedOffset.x <= -0)
                    grassPb = GrassTopComp::colliderLeft + trans.m_pos;
                else
                    continue;
                    
                auto pb = phys2.pushbox + trans2.m_pos;
                auto res = pb.checkOverlap(grassPb);
                if ((res & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH)
                    grass.touchedPlayer(avgAppliedOffset, {.reg=&m_reg, .idx=idx});
            }
        }
    }
}

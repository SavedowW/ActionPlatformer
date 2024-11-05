#include "BattleSystem.h"
#include "StateMachine.h"
#include "Profile.h"

BattleSystem::BattleSystem(entt::registry &reg_, Application &app_, Camera &cam_) :
    m_reg(reg_), m_app(app_), m_cam(cam_)
{
}

void BattleSystem::update()
{
    PROFILE_FUNCTION;

    auto viewBtl = m_reg.view<BattleActor, StateMachine>();
    std::set<uint32_t> newPresentHits;

    for (auto [idx, btl, sm] : viewBtl.each())
    {
        dynamic_cast<PhysicalState*>(sm.getRealCurrentState())->updateActor(btl);
        for (const auto &atk : btl.m_activeHits)
            newPresentHits.insert(atk->m_hitData.m_id);
    }

    for (const auto &id : m_presentHits)
    {
        if (!newPresentHits.contains(id))
        {
            for (auto [idx, btl, sm] : viewBtl.each())
                btl.m_appliedHits.erase(id);
        }
    }

    m_presentHits = std::move(newPresentHits);
}

void BattleSystem::handleAttacks()
{
    PROFILE_FUNCTION;

    auto viewBtl = m_reg.view<BattleActor, StateMachine, ComponentTransform>();
    for (auto [idx, btl, sm, trans] : viewBtl.each())
    {
        for (auto *atk : btl.m_activeHits)
        {
            for (auto [idxVic, btlVic, smVic, transVic] : viewBtl.each())
            {
                if (idx == idxVic || atk->m_hitData.m_friendTeams.contains(btlVic.m_team))
                    continue;

                if (btlVic.m_appliedHits.contains(atk->m_hitData.m_id))
                    continue;

                for (const auto &hurtGroup : *btlVic.m_hurtboxes)
                {
                    auto hitidef = detectHit(atk->m_colliders, btl.m_currentFrame, trans, hurtGroup.m_colliders, btlVic.m_currentFrame, transVic);
                    if (hitidef.m_hitOccured)
                    {
                        btlVic.m_appliedHits.insert(atk->m_hitData.m_id);
                        m_appliedHits.push(hitidef.m_hitPos);
                        applyHit({idx, btl, sm, trans}, {idxVic, btlVic, smVic, transVic}, *atk);
                        break;
                    }
                }
            }
        }
    }
}

void BattleSystem::debugDraw()
{
    auto &rnd = *m_app.getRenderer();
    for (int i = 0; i < m_appliedHits.getFilled(); ++i)
    {
        const auto &el = m_appliedHits[i];
        rnd.drawCross(el, {1.0f, 5.0f}, {5.0f, 1.0f}, {0, 0, 0, 255}, m_cam);
    }
}

void BattleSystem::applyHit(ActorDescr attacker_, ActorDescr victim_, const HitboxGroup &hit_)
{
    if (hit_.m_hitData.m_hitstop)
    {
        auto *physAtk = m_reg.try_get<ComponentPhysical>(attacker_.m_id);
        auto *vicAtk = m_reg.try_get<ComponentPhysical>(victim_.m_id);

        if (physAtk)
            physAtk->m_hitstopLeft = hit_.m_hitData.m_hitstop;

        if (vicAtk)
            vicAtk->m_hitstopLeft = hit_.m_hitData.m_hitstop;
    }

    if (hit_.m_hitData.m_victimFlash)
    {
        auto *vicRen = m_reg.try_get<ComponentAnimationRenderable>(victim_.m_id);
        vicRen->m_flash = hit_.m_hitData.m_victimFlash->clone();
    }

    if (hit_.m_hitData.m_onHitShake.m_period > 0)
        m_cam.startShake(hit_.m_hitData.m_onHitShake.m_xAmp, hit_.m_hitData.m_onHitShake.m_yAmp, hit_.m_hitData.m_onHitShake.m_period);

    if (victim_.m_actor.m_hitStateTransitions && !victim_.m_actor.m_hitStateTransitions->m_hitstunTransitions.isEmpty())
        victim_.m_sm.switchCurrentState({&m_reg, victim_.m_id}, victim_.m_actor.m_hitStateTransitions->m_hitstunTransitions[hit_.m_hitData.m_hitstun]);
}

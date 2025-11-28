#include "BattleSystem.h"
#include "StateMachine.h"
#include "Core/Profile.h"
#include "Core/Application.h"

BattleSystem::BattleSystem(entt::registry &reg_, Camera &cam_) :
    m_reg(reg_),
    m_cam(cam_)
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
                    auto hitidef = detectHit(atk->m_colliders, btl.m_timeInState, trans, hurtGroup.m_colliders, btlVic.m_timeInState, transVic);
                    if (hitidef.m_hitOccured)
                    {
                        btlVic.m_appliedHits.insert(atk->m_hitData.m_id);
                        m_appliedHits.push(hitidef.m_hitPos);
                        applyHit({.m_id=idx, .m_actor=btl, .m_sm=sm, .m_trans=trans}, {.m_id=idxVic, .m_actor=btlVic, .m_sm=smVic, .m_trans=transVic}, *atk);
                        break;
                    }
                }
            }
        }
    }
}

void BattleSystem::debugDraw() const
{
    auto &rnd = Application::instance().m_renderer;
    for (size_t i = 0; i < m_appliedHits.getFilled(); ++i)
    {
        const auto &el = m_appliedHits[i];
        rnd.drawCross(el, {1, 5}, {5, 1}, {0, 0, 0, 255}, m_cam);
    }
}

void BattleSystem::applyHit(ActorDescr attacker_, ActorDescr victim_, const HitboxGroup &hit_)
{
    // Inflict damage and trigger related animation
    if (auto *hren = m_reg.try_get<HealthRendererCommonWRT>(victim_.m_id))
        hren->touch();

    if (hit_.m_hitData.m_damage)
    {
        auto *vicHP = m_reg.try_get<HealthOwner>(victim_.m_id);

        if (vicHP)
        {
            const auto newhealth = vicHP->takeDamage(hit_.m_hitData.m_damage);

            if (auto *hren = m_reg.try_get<HealthRendererCommonWRT>(victim_.m_id))
                hren->takeDamage(newhealth);
        }
    }

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

    if (hit_.m_hitData.m_onHitShake.m_period > Time::NS{0})
        m_cam.startShake(hit_.m_hitData.m_onHitShake.m_xAmp, hit_.m_hitData.m_onHitShake.m_yAmp, hit_.m_hitData.m_onHitShake.m_period);

    if (victim_.m_actor.m_hitStateTransitions && !victim_.m_actor.m_hitStateTransitions->m_hitstunTransitions.isEmpty())
        victim_.m_sm.switchCurrentState({&m_reg, victim_.m_id}, victim_.m_actor.m_hitStateTransitions->m_hitstunTransitions[hit_.m_hitData.m_hitstun]);
}

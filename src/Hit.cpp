#include "Hit.h"

uint32_t Hit::m_lastId = 0;

BattleActor::BattleActor(BattleTeams team_) :
    m_team(team_)
{
}

HitboxGroup HitGeneration::hitPlayerLight()
{
    HitboxGroup hit;

    hit.m_hitData.m_damage = 0;
    hit.m_hitData.m_hitstun = 2;
    hit.m_hitData.m_stagger = 10.0f;
    hit.m_hitData.m_friendTeams.insert(BattleTeams::PLAYER);
    hit.m_hitData.m_hitstop = 8;
    hit.m_hitData.m_victimFlash = std::make_unique<FlashLinear>(10, SDL_Color{255, 255, 255, 255}, 0);
    hit.m_hitData.m_onHitShake = {5, 5, 8};

    hit.m_colliders.push_back({
        {{5.0f, -15.0f}, {8.0f, 4.0f}},
        TimelineProperty<bool>({{5, true}, {10, false}})
    });

    hit.m_colliders.push_back({
        {{22.0f, -21.0f}, {15.0f, 9.0f}},
        TimelineProperty<bool>({{5, true}, {10, false}})
    });

    hit.m_activeWindow = {5, 11};


    return hit;
}

HitboxGroup HitGeneration::hitPlayerChain()
{
    HitboxGroup hit;

    hit.m_hitData.m_damage = 1;
    hit.m_hitData.m_hitstun = 3;
    hit.m_hitData.m_stagger = 7.0f;
    hit.m_hitData.m_friendTeams.insert(BattleTeams::PLAYER);
    hit.m_hitData.m_hitstop = 16;
    hit.m_hitData.m_victimFlash = std::make_unique<FlashLinear>(10, SDL_Color{255, 255, 255, 255}, 0);

    hit.m_colliders.push_back({
        {{26.0f, -16.0f}, {24.0f, 16.0f}},
        TimelineProperty<bool>({{12, true}, {15, false}})
    });

    hit.m_activeWindow = {12, 15};


    return hit;
}

void Hit::updateId()
{
    m_id = ++m_lastId;
}

HitPosResult detectHit(const std::vector<TemporaryCollider> &hit_, uint32_t hitActiveFrame_, const ComponentTransform &attacker_, const std::vector<TemporaryCollider> &hurtbox_, uint32_t hurtboxActiveFrame_, const ComponentTransform &victim_)
{
    utils::Average<Vector2<float>> avgpos;

    for (const auto &hitbox : hit_)
    {
        if (!hitbox.m_timeline[hitActiveFrame_])
            continue;
        
        auto hitboxCld = getColliderAt(hitbox.m_collider, attacker_);

        for (const auto &hurtbox : hurtbox_)
        {
            if (!hurtbox.m_timeline[hurtboxActiveFrame_])
                continue;

            auto hurtboxCld = getColliderAt(hurtbox.m_collider, victim_);

            auto overlap = hitboxCld.getOverlapArea(hurtboxCld);
            if (overlap.m_halfSize.x > 0 && overlap.m_halfSize.y > 0)
                avgpos += overlap.m_center;
        }
    }

    if (avgpos.isSet())
        return {true, avgpos};
    else
        return {false};
}

HitStateMapping &HitStateMapping::addHitstunTransition(uint32_t level_, CharState transition_)
{
    m_hitstunTransitions.addPropertyValue(level_, CharState{transition_});
    return *this;
}

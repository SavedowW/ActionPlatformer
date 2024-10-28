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

    hit.m_colliders.push_back({
        {{9.0f, -12.0f}, {4.0f, 4.0f}},
        TimelineProperty<bool>({{5, true}, {12, false}})
    });

    hit.m_colliders.push_back({
        {{22.0f, -15.0f}, {12.0f, 2.5f}},
        TimelineProperty<bool>({{5, true}, {8, false}})
    });

    hit.m_colliders.push_back({
        {{14.0f, -14.0f}, {8.0f, 2.0f}},
        TimelineProperty<bool>({{8, true}, {11, false}})
    });

    hit.m_activeWindow = {5, 11};


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

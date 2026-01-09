#include "Hit.h"
#include "Core/Application.h"

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
    hit.m_hitData.m_victimFlash = std::make_unique<FlashDelayedLinear>(8, 10, 0);
    hit.m_hitData.m_onHitShake = {5, 5, 8};

    hit.m_colliders.push_back({
        {{-3, -19}, {16, 8}},
        TimelineProperty<bool>({{5, true}, {10, false}})
    });

    hit.m_colliders.push_back({
        {{7, -30}, {30, 18}},
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
    hit.m_hitData.m_victimFlash = std::make_unique<FlashDelayedLinear>(16, 10, 0);

    hit.m_colliders.push_back({
        {{-24, -32}, {48, 24}},
        TimelineProperty<bool>({{12, true}, {13, false}})
    });

    hit.m_colliders.push_back({
        {{2, -32}, {48, 32}},
        TimelineProperty<bool>({{12, true}, {15, false}})
    });

    hit.m_activeWindow = {12, 15};


    return hit;
}

HitboxGroup HitGeneration::hitPlayerAirAttack()
{
    HitboxGroup hit;

    hit.m_hitData.m_damage = 0;
    hit.m_hitData.m_hitstun = 2;
    hit.m_hitData.m_stagger = 10.0f;
    hit.m_hitData.m_friendTeams.insert(BattleTeams::PLAYER);
    hit.m_hitData.m_hitstop = 8;
    hit.m_hitData.m_victimFlash = std::make_unique<FlashDelayedLinear>(8, 10, 0);
    hit.m_hitData.m_onHitShake = {5, 5, 8};

    hit.m_colliders.push_back({
        {{-16, -64}, {56, 98}},
        TimelineProperty<bool>({{8, true}, {15, false}})
    });

    hit.m_colliders.push_back({
        {{26, -49}, {25, 60}},
        TimelineProperty<bool>({{8, true}, {15, false}})
    });

    hit.m_activeWindow = {8, 15};


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
            if (overlap.m_size.x > 0 && overlap.m_size.y > 0)
                avgpos += overlap.m_topLeft + overlap.m_size / 2.0f;
        }
    }

    if (avgpos.isSet())
        return {true, avgpos};
    else
        return {false};
}

HitStateMapping &HitStateMapping::addHitstunTransition(uint32_t level_, CharState transition_)
{
    m_hitstunTransitions.addPair(level_, CharState{transition_});
    return *this;
}

HealthOwner::HealthOwner(uint8_t realHealth_):
    m_realHealth(realHealth_)
{
}

uint8_t HealthOwner::takeDamage(uint8_t damage_)
{
    if (m_realHealth > damage_)
        m_realHealth -= damage_;
    else
        m_realHealth = 0;
    
    return m_realHealth;
}

HealthRendererCommonWRT::HealthRendererCommonWRT(uint8_t realHealth_, const Vector2<int> &offset_) :
    m_realHealth(realHealth_),
    m_offset(offset_)
{
    auto &animmgmt = Application::instance().m_animationManager;
    for (auto i = 0u; i < realHealth_; ++i)
    {
        m_heartAnims.emplace_back(animmgmt, animmgmt.getAnimID("UI/heart"), LOOPMETHOD::NOLOOP);
    }
}

void HealthRendererCommonWRT::update()
{
    switch (m_state)
    {
        case DelayFadeStates::INACTIVE:
            break;

        case DelayFadeStates::FADE_IN:
            if (m_delayFadeTimer.update())
            {
                m_state = DelayFadeStates::ANIMATION;
            }
            break;

        case DelayFadeStates::ANIMATION:
        {

            bool hasActiveAnimations = false;

            for (size_t i = m_realHealth; i < static_cast<uint8_t>(m_heartAnims.size()); ++i)
            {
                if (!m_heartAnims[i].isFinished())
                    hasActiveAnimations = true;
                    
                m_heartAnims[i].update();
            }

            if (!hasActiveAnimations)
            {
                m_state = DelayFadeStates::IDLE;
                m_delayFadeTimer.begin(180u);
            }

            break;
        }

        case DelayFadeStates::IDLE:
            if (m_delayFadeTimer.update())
            {
                m_state = DelayFadeStates::FADE_OUT;
                m_delayFadeTimer.begin(60);
            }
            break;

        case DelayFadeStates::FADE_OUT:
            if (m_delayFadeTimer.update())
            {
                m_state = DelayFadeStates::INACTIVE;
                m_heartAnims.erase(m_heartAnims.begin() + m_realHealth, m_heartAnims.end());
            }
            break;
    }
}

void HealthRendererCommonWRT::takeDamage(uint8_t newHealth_)
{
    m_realHealth = newHealth_;
    touch();
}

void HealthRendererCommonWRT::touch()
{
    switch (m_state)
    {
        case DelayFadeStates::INACTIVE:
            // Begin fade in timer
            m_delayFadeTimer.begin(10);
            m_state = DelayFadeStates::FADE_IN;
            break;

        case DelayFadeStates::IDLE:
            // Might have taken damage, otherwise it will just reset the timer
            m_state = DelayFadeStates::ANIMATION;
            break;

        case DelayFadeStates::FADE_OUT:
            // Go from fade out into fade in timer with same progress (alpha)
            m_delayFadeTimer.beginAt(10, 1.0f - m_delayFadeTimer.getProgressNormalized());
            m_state = DelayFadeStates::FADE_IN;
            break;

        default:
            // Already in fade in or active animation, no need to update state
            break;
    }
}

T_NAME_AUTO(BattleActor);
T_NAME_AUTO(HealthOwner);
T_NAME_AUTO(HealthRendererCommonWRT);

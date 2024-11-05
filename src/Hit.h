#ifndef HIT_H_
#define HIT_H_
#include "TimelineProperty.hpp"
#include "RectCollider.h"
#include "CoreComponents.h"
#include "StateCommon.h"
#include <set>

enum class HurtTrait {
    VULNERABLE,
    NORMAL
};

enum class BattleTeams {
    PLAYER,
    ENEMIES,
    NONE
};

struct TemporaryCollider
{
    Collider m_collider;
    TimelineProperty<bool> m_timeline;
};

struct HurtboxGroup
{
    std::vector<TemporaryCollider> m_colliders;
    HurtTrait m_trait;    
};

using Hurtbox = std::vector<HurtboxGroup>;

struct CamShakeDescr
{
    int m_xAmp = 0;
    int m_yAmp = 0;
    uint32_t m_period = 0;
};

/*
    Hit data:
    Damage - only a few attacks deal damage, health is limited by a small amount of hearts (approx 3-10 for most enemies)
    Hitstun - level of hitstun, the higher the value, the higher the chance for the actor to get stunned.
        BattleActor might or might not have different duration or behavior for different hitstun levels
    Stagger - actual damage. Every enemy has a stagger gauge that is constantly passively depleted
        but raises from stagger damage. Once the gauge is full, the actor is staggered
    Friend teams - every actor has an assigned team, hits are not applied to the members of friendly teams
    Hitstop - applied to both sides
*/

struct Hit
{
    static uint32_t m_lastId;
    uint32_t m_id = 0;

    int m_damage = false;
    uint32_t m_hitstun = 0;
    float m_stagger = 0.0f;
    uint32_t m_hitstop = 0;
    std::set<BattleTeams> m_friendTeams;
    std::unique_ptr<Flash> m_victimFlash;

    CamShakeDescr m_onHitShake;
    
    void updateId();
};

struct HitboxGroup
{
    std::vector<TemporaryCollider> m_colliders;
    Hit m_hitData;
    std::pair<uint32_t, uint32_t> m_activeWindow;
};

struct HitPosResult
{
    bool m_hitOccured;
    Vector2<float> m_hitPos;
};

HitPosResult detectHit(const std::vector<TemporaryCollider> &hit_, uint32_t hitActiveFrame_, const ComponentTransform &attacker_, const std::vector<TemporaryCollider> &hurtbox_, uint32_t hurtboxActiveFrame_, const ComponentTransform &victim_);

struct HitStateMapping
{
    TimelineProperty<CharState> m_hitstunTransitions;

    HitStateMapping &addHitstunTransition(uint32_t level_, CharState transition_);
};

struct BattleActor
{
    BattleActor(BattleTeams team_);
    const Hurtbox *m_hurtboxes = nullptr;
    uint32_t m_currentFrame = 0;
    BattleTeams m_team = BattleTeams::NONE;
    std::vector<const HitboxGroup *> m_activeHits;
    std::set<uint32_t> m_appliedHits;
    const HitStateMapping *m_hitStateTransitions = nullptr;
};

namespace HitGeneration
{

HitboxGroup hitPlayerLight();

}

#endif

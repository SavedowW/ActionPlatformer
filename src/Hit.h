#ifndef HIT_H_
#define HIT_H_
#include "Core/AnimationManager.h"
#include "Core/TimelineProperty.hpp"
#include "Core/RectCollider.h"
#include "Core/CoreComponents.h"
#include "Core/StateCommon.h"
#include "Core/ManualTimer.h"
#include <set>

enum class HurtTrait : uint8_t {
    VULNERABLE,
    NORMAL
};

enum class BattleTeams : uint8_t {
    PLAYER,
    ENEMIES,
    NONE
};

struct TemporaryCollider
{
    Collider m_collider;
    TimelineProperty<Time::NS, bool> m_timeline;
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
    Time::NS m_period{0};
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
    std::pair<Time::NS, Time::NS> m_activeWindow;
};

struct HitPosResult
{
    bool m_hitOccured = false;
    Vector2<float> m_hitPos{};
};

HitPosResult detectHit(const std::vector<TemporaryCollider> &hit_, const Time::NS &hitActiveTime_, const ComponentTransform &attacker_, const std::vector<TemporaryCollider> &hurtbox_, const Time::NS &hurtboxActiveTime_, const ComponentTransform &victim_);

struct HitStateMapping
{
    TimelineProperty<uint32_t, CharState> m_hitstunTransitions;

    HitStateMapping &addHitstunTransition(uint32_t level_, CharState transition_);
};

struct BattleActor
{
    BattleActor(BattleTeams team_);
    const Hurtbox *m_hurtboxes = nullptr;
    Time::NS m_timeInState{0};
    BattleTeams m_team = BattleTeams::NONE;
    std::vector<const HitboxGroup *> m_activeHits;
    std::set<uint32_t> m_appliedHits;
    const HitStateMapping *m_hitStateTransitions = nullptr;
};

struct HealthOwner
{
    HealthOwner(uint8_t realHealth_);
    HealthOwner(const HealthOwner &rhs_) = delete;
    HealthOwner(HealthOwner &&rhs_) = default;
    HealthOwner &operator=(const HealthOwner &rhs_) = delete;
    HealthOwner &operator=(HealthOwner &&rhs_) = default;

    uint8_t takeDamage(uint8_t damage_);

    uint8_t m_realHealth = 0;
};

// In world, relative to transform, on timers
struct HealthRendererCommonWRT
{
    HealthRendererCommonWRT(uint8_t realHealth_, const Vector2<int> &offset_);
    HealthRendererCommonWRT(const HealthRendererCommonWRT &rhs_) = delete;
    HealthRendererCommonWRT(HealthRendererCommonWRT &&rhs_) = default;
    HealthRendererCommonWRT &operator=(const HealthRendererCommonWRT &rhs_) = delete;
    HealthRendererCommonWRT &operator=(HealthRendererCommonWRT &&rhs_) = default;

    void update(const Time::NS &frameTime_);
    void takeDamage(uint8_t newHealth_);
    void touch();

    std::vector<Animation> m_heartAnims;
    uint8_t m_realHealth = 0;
    Vector2<int> m_offset;

    enum class DelayFadeStates : uint8_t
    {
        INACTIVE,
        FADE_IN,
        ANIMATION,
        IDLE,
        FADE_OUT
    } m_state = DelayFadeStates::INACTIVE;
    ManualTimer<false> m_delayFadeTimer;
};

namespace HitGeneration
{

HitboxGroup hitPlayerLight();
HitboxGroup hitPlayerChain();
HitboxGroup hitPlayerAirAttack();

}

#endif

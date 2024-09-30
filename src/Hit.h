#ifndef HIT_H_
#define HIT_H_
#include "TimelineProperty.hpp"
#include "RectCollider.h"

enum class HurtTrait {
    VULNERABLE,
    NORMAL
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


struct Hit
{
    int m_damage = false;
    int m_hitstun = 0;
    float m_stagger = 0.0f;
};

struct HitboxGroup
{
    std::vector<TemporaryCollider> m_colliders;
    Hit m_hitData;
};

struct BattleActor
{
    const Hurtbox *m_hurtboxes = nullptr;
    uint32_t m_currentFrame = 0;
};

#endif

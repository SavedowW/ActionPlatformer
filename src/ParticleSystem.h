#pragma once
#include "Core/CoreComponents.h"
#include "Core/AnimationManager.h"
#include "Core/Vector2.hpp"
#include "Core/ECS/ComponentsView.h"
#include <entt/entt.hpp>
#include <SDL3/SDL.h>

enum class TiePosRule : uint8_t
{
    TIE_TO_EMITTER,
    TIE_TO_GROUND,
    TIE_TO_WALL,
    NONE
};

class ParticleRecipe
{
public:
    ParticleRecipe(ResID anim_, uint32_t lifetime_, int layer_);
    ParticleRecipe(const ParticleRecipe&) = default;
    ParticleRecipe &setOffset(const Vector2<int> &offset_) noexcept;
    ParticleRecipe &setFlip(ParticleFlip flip_);
    ParticleRecipe &tiePos(TiePosRule rule_) noexcept;

private:
    Vector2<int> offset;
    ResID anim;
    uint32_t lifetime = 0;
    int layer;
    TiePosRule tiePosRule = TiePosRule::NONE;
    Flag<ParticleFlip> flip = ParticleFlip::NONE;

    friend class ParticleSystem;
};

struct ParticleEmissionRuleset
{
    ParticleEmissionRuleset(const ParticleRecipe &recipe_);
    ParticleEmissionRuleset &destroyOnStateChange() noexcept;

    ParticleRecipe recipe;
    bool mustDestroyOnStateChange = false;
};

class ParticleSystem
{
public:
    ParticleSystem(entt::registry &reg_);

    template<IsComponentsView ViewT>
    entt::entity makeParticle(const ParticleRecipe &particle_, const ViewT &view_);

    void update();

private:
    // These methods ignore tie rule in the recipe
    entt::entity MakeParticleUntied(
        const ParticleRecipe &particle_,
        const ComponentTransform &emitterTrans_
    );
    entt::entity MakeParticleTiedEmitter(
        const ParticleRecipe &particle_,
        const ComponentTransform &emitterTrans_,
        entt::entity emitterPid_
    );
    entt::entity MakeParticleTiedCollider(
        const ParticleRecipe &particle_,
        const ComponentTransform &emitterTrans_,
        entt::entity colliderPid_
    );

    entt::entity MakeParticleTiedColliderImpl(
        const ParticleRecipe &particle_,
        const ComponentTransform &emitterTrans_,
        entt::entity colliderPid_,
        const ComponentTransform &colliderTrans_,
        const ComponentStaticCollider &colliderCollider_
    );

    entt::registry &m_registry;
    AnimationManager &m_animmgmt;

};

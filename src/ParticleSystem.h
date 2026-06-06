#pragma once
#include "Core/AnimationManager.h"
#include "Core/Vector2.hpp"
#include "Core/ECS/ComponentsView.h"
#include <entt/entt.hpp>
#include <SDL3/SDL.h>

enum class TiePosRule : uint8_t
{
    TIE_TO_EMITTER,
    NONE
};

class ParticleRecipe
{
public:
    ParticleRecipe(ResID anim_, uint32_t lifetime_, int layer_);
    ParticleRecipe &setOffset(const Vector2<int> &offset_) noexcept;
    ParticleRecipe &tiePos(TiePosRule rule_) noexcept;

private:
    Vector2<int> offset;
    ResID anim;
    uint32_t lifetime = 0;
    int layer;
    TiePosRule tiePosRule = TiePosRule::NONE;

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
    entt::registry &m_registry;
    AnimationManager &m_animmgmt;

};

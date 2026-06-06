#include "ParticleSystem.h"
#include "Core/CoreComponents.h"
#include "Core/Application.h"

ParticleRecipe::ParticleRecipe(ResID anim_, uint32_t lifetime_, int layer_) :
    anim{anim_},
    lifetime{lifetime_},
    layer{layer_}
{}

ParticleRecipe &ParticleRecipe::setOffset(const Vector2<int> &offset_) noexcept
{
    offset = offset_;
    return *this;
}

ParticleRecipe &ParticleRecipe::tiePos(TiePosRule rule_) noexcept
{
    tiePosRule = rule_;
    return *this;
}

ParticleEmissionRuleset::ParticleEmissionRuleset(const ParticleRecipe &recipe_) :
    recipe{recipe_}
{}

ParticleEmissionRuleset &ParticleEmissionRuleset::destroyOnStateChange() noexcept
{
    mustDestroyOnStateChange = true;
    return *this;
}

ParticleSystem::ParticleSystem(entt::registry &reg_) :
    m_registry(reg_),
    m_animmgmt(Application::instance().m_animationManager)
{
}

void ParticleSystem::update()
{
    auto viewParticles = m_registry.view<ComponentParticlePrimitive>();

    for (const auto &[idx, pprim] : viewParticles.each())
    {
        if (pprim.lifetime.update())
            m_registry.destroy(idx);
    }
}

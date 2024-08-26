#include "ParticleSystem.h"
#include "CoreComponents.h"

ParticleSystem::ParticleSystem(entt::registry &reg_, Application &app_) :
    m_registry(reg_),
    m_animmgmt(*app_.getAnimationManager())
{
}

void ParticleSystem::makeParticle(const ParticleRecipe &particle_)
{
    EXPECTED_RENDER_LAYERS(3);
    
    switch (particle_.layer)
    {
        case 0:
            makeParticle<0>(particle_);
            break;

        case 1:
            makeParticle<1>(particle_);
            break;

        case 2:
            makeParticle<2>(particle_);
            break;

        default:
            throw std::string("Particle has an incorrect layer: ") + std::to_string(particle_.layer);
    }
}

template<size_t LAYER>
void ParticleSystem::makeParticle(const ParticleRecipe &particle_)
{
    for (int i = 0; i < particle_.count; ++i)
    {
        auto pid = m_registry.create();
        m_registry.emplace<ComponentTransform>(pid, particle_.pos, ORIENTATION::UNSPECIFIED);
        auto &pprim = m_registry.emplace<ComponentParticlePrimitive>(pid, particle_.flip);
        m_registry.emplace<ComponentParticlePhysics>(pid);
    
        auto &animrnd = m_registry.emplace<ComponentAnimationRenderable>(pid);
        m_registry.emplace<RenderLayer<LAYER>>(pid);
        animrnd.m_animations[particle_.anim] = std::make_unique<Animation>(m_animmgmt, particle_.anim, LOOPMETHOD::JUMP_LOOP);
        animrnd.m_currentAnimation = animrnd.m_animations[particle_.anim].get();

        if (particle_.lifetime)
            pprim.m_lifetime.begin(particle_.lifetime);
        
        pprim.angle = particle_.angle;
    }
}

void ParticleSystem::update()
{
    auto viewParticles = m_registry.view<ComponentParticlePrimitive>();

    for (auto [idx, pprim] : viewParticles.each())
    {
        if (pprim.m_lifetime.update())
            m_registry.destroy(idx);
    }
}

ParticleTemplate::ParticleTemplate(int count_, const Vector2<float> &offset_, int anim_, uint32_t lifetime_, size_t layer_, utils::Gate<float> &&horizontalFlipGate_, utils::Gate<float> &&verticalFlipGate_) :
    count(count_),
    offset(offset_),
    anim(anim_),
    lifetime(lifetime_),
    layer(layer_),
    horizontalFlipGate(std::forward<utils::Gate<float>>(horizontalFlipGate_)),
    verticalFlipGate(std::forward<utils::Gate<float>>(verticalFlipGate_))
{
}

ParticleRecipe::ParticleRecipe(const ParticleTemplate &template_) :
    count(template_.count),
    anim(template_.anim),
    lifetime(template_.lifetime),
    layer(template_.layer)
{
}

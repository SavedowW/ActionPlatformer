#include "ParticleSystem.h"
#include "CoreComponents.h"

ParticleSystem::ParticleSystem(entt::registry &reg_, Application &app_) :
    m_registry(reg_),
    m_animmgmt(*app_.getAnimationManager())
{
}

void ParticleSystem::makeParticle(const ParticleRecipe &particle_)
{
    for (int i = 0; i < particle_.count; ++i)
    {
        auto pid = m_registry.create();
        m_registry.emplace<ComponentTransform>(pid, particle_.pos, ORIENTATION::UNSPECIFIED);
        auto &pprim = m_registry.emplace<ComponentParticlePrimitive>(pid, particle_.flip);
        m_registry.emplace<ComponentParticlePhysics>(pid);
    
        auto &animrnd = m_registry.emplace<ComponentAnimationRenderable>(pid);
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

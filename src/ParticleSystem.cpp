#include "ParticleSystem.h"
#include "CoreComponents.h"
#include "Profile.h"

ParticleSystem::ParticleSystem(entt::registry &reg_, Application &app_) :
    m_registry(reg_),
    m_animmgmt(*app_.getAnimationManager())
{
}

void ParticleSystem::makeParticle(const ParticleRecipe &particle_, std::vector<entt::entity> *placeId_)
{
    EXPECTED_RENDER_LAYERS(3);
    
    switch (particle_.layer)
    {
        case 0:
            makeParticle<0>(particle_, placeId_);
            break;

        case 1:
            makeParticle<1>(particle_, placeId_);
            break;

        case 2:
            makeParticle<2>(particle_, placeId_);
            break;

        default:
            throw std::string("Particle has an incorrect layer: ") + std::to_string(particle_.layer);
    }
}

template<size_t LAYER>
void ParticleSystem::makeParticle(const ParticleRecipe &particle_, std::vector<entt::entity> *placeId_)
{
    for (int i = 0; i < particle_.count; ++i)
    {
        auto pid = m_registry.create();
        auto &trans = m_registry.emplace<ComponentTransform>(pid, particle_.pos, ORIENTATION::UNSPECIFIED);
        auto &pprim = m_registry.emplace<ComponentParticlePrimitive>(pid, particle_.flip);
        auto &phys = m_registry.emplace<ComponentParticlePhysics>(pid);
        if (particle_.m_tiePosTo != entt::null)
        {
            pprim.m_tieTransform = particle_.m_tiePosTo;

            if (particle_.m_baseTemplate.m_tiePosRule != TiePosRule::TIE_TO_SOURCE)
                trans.m_pos -= m_registry.get<ComponentTransform>(particle_.m_tiePosTo).m_pos;
        }
    
        auto &animrnd = m_registry.emplace<ComponentAnimationRenderable>(pid);
        m_registry.emplace<RenderLayer<LAYER>>(pid);
        animrnd.m_animations[particle_.anim] = std::make_unique<Animation>(m_animmgmt, particle_.anim, LOOPMETHOD::JUMP_LOOP);
        animrnd.m_currentAnimation = animrnd.m_animations[particle_.anim].get();

        if (particle_.lifetime)
            pprim.m_lifetime.begin(particle_.lifetime);
        
        pprim.angle = particle_.angle;

        if (placeId_)
            placeId_->push_back(pid);
    }
}

void ParticleSystem::update()
{
    PROFILE_FUNCTION;

    auto viewParticles = m_registry.view<ComponentParticlePrimitive>();

    for (auto [idx, pprim] : viewParticles.each())
    {
        if (pprim.m_lifetime.update())
            m_registry.destroy(idx);
    }
}

ParticleTemplate::ParticleTemplate(int count_, const Vector2<float> &offset_, int anim_, uint32_t lifetime_, size_t layer_) :
    count(count_),
    offset(offset_),
    anim(anim_),
    lifetime(lifetime_),
    layer(layer_)
{
}

ParticleTemplate &ParticleTemplate::setTiePosRules(TiePosRule tiePosRule_)
{
    m_tiePosRule = tiePosRule_;
    return *this;
}

ParticleTemplate &ParticleTemplate::setTieLifetimeRules(TieLifetimeRule tieRule_)
{
    m_tieLifetimeRule = tieRule_;
    return *this;
}

ParticleTemplate &ParticleTemplate::setNotDependOnGroundAngle()
{
    m_dependOnGroundAngle = false;
    return *this;
}

ParticleRecipe::ParticleRecipe(const ParticleTemplate &template_) :
    count(template_.count),
    anim(template_.anim),
    lifetime(template_.lifetime),
    layer(template_.layer),
    m_baseTemplate(template_)
{
}

#include "ParticleSystem.h"
#include "CoreComponents.h"
#include "Profile.h"
#include "Application.h"

ParticleSystem::ParticleSystem(entt::registry &reg_) :
    m_registry(reg_),
    m_animmgmt(Application::instance().m_animationManager)
{
}

void ParticleSystem::makeParticle(const ParticleRecipe &particle_, std::vector<entt::entity> *placeId_)
{
    for (int i = 0; i < particle_.count; ++i)
    {
        auto pid = m_registry.create();
        auto &trans = m_registry.emplace<ComponentTransform>(pid, particle_.pos, ORIENTATION::UNSPECIFIED);
        auto &pprim = m_registry.emplace<ComponentParticlePrimitive>(pid, particle_.flip);
        if (particle_.m_tiePosTo != entt::null)
        {
            pprim.m_tieTransform = particle_.m_tiePosTo;

            if (particle_.m_baseTemplate.m_tiePosRule != TiePosRule::TIE_TO_SOURCE)
                trans.m_pos -= m_registry.get<ComponentTransform>(particle_.m_tiePosTo).m_pos;
        }
    
        auto &animrnd = m_registry.emplace<ComponentAnimationRenderable>(pid);
        m_registry.emplace<RenderLayer>(pid, particle_.layer);
        animrnd.loadAnimation(m_animmgmt, particle_.anim);
        animrnd.m_currentAnimation = &animrnd.m_animations.at(particle_.anim);

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

ParticleTemplate::ParticleTemplate(int count_, const Vector2<float> &offset_, ResID anim_, uint32_t lifetime_, int layer_) :
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

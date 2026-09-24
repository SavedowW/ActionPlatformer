#include "ParticleSystem.h"
#include "Core/CoreComponents.h"
#include "Core/Application.h"
#include "Core/Logger.hpp"
#include <numbers>

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

ParticleRecipe &ParticleRecipe::setFlip(ParticleFlip flip_)
{
    flip |= flip_;
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

entt::entity ParticleSystem::MakeParticleUntied(
    const ParticleRecipe &particle_,
    const ComponentTransform &emitterTrans_)
{
    auto pid = m_registry.create();

    auto offset = particle_.offset;
    if (emitterTrans_.m_orientation == Orientation::LEFT)
        offset.x *= -1;

    m_registry.emplace<ComponentTransform>(pid, emitterTrans_.m_pos + offset, emitterTrans_.m_orientation);

    auto &prim = m_registry.emplace<ComponentParticlePrimitive>(pid);
    prim.lifetime.begin(particle_.lifetime);
    prim.flip = particle_.flip;

    auto &animrnd = m_registry.emplace<ComponentAnimationRenderable>(pid);
    animrnd.loadAnimation(m_animmgmt, particle_.anim);
    animrnd.m_currentAnimation = &animrnd.m_animations.at(particle_.anim);

    m_registry.emplace<RenderLayer>(pid, particle_.layer);

    return pid;
}

entt::entity ParticleSystem::MakeParticleTiedEmitter(
    const ParticleRecipe &particle_,
    const ComponentTransform &emitterTrans_,
    const entt::entity emitterPid_)
{
    auto pid = m_registry.create();

    m_registry.emplace<ComponentTransform>(pid, particle_.offset, emitterTrans_.m_orientation);

    auto &prim = m_registry.emplace<ComponentParticlePrimitive>(pid);
    prim.lifetime.begin(particle_.lifetime);
    prim.tieTransform = emitterPid_;
    prim.flip = particle_.flip;

    auto &animrnd = m_registry.emplace<ComponentAnimationRenderable>(pid);
    animrnd.loadAnimation(m_animmgmt, particle_.anim);
    animrnd.m_currentAnimation = &animrnd.m_animations.at(particle_.anim);

    m_registry.emplace<RenderLayer>(pid, particle_.layer);

    return pid;
}

entt::entity ParticleSystem::MakeParticleTiedCollider(
    const ParticleRecipe &particle_,
    const ComponentTransform &emitterTrans_,
    const entt::entity colliderPid_)
{
    if (colliderPid_ != entt::null)
    {
        if (auto *transGround = m_registry.try_get<ComponentTransform>(colliderPid_))
        {
            if (auto *colliderGround = m_registry.try_get<ComponentStaticCollider>(colliderPid_))
            {
                return MakeParticleTiedColliderImpl(
                    particle_,
                    emitterTrans_,
                    colliderPid_,
                    *transGround,
                    *colliderGround
                );
            }
        }
    }

    // Last resort - tie to emitter
    LOG_WARNING("Creating particle at {} {} as tied to collider ({}), but collider most likely was not found, making untied instead", particle_.offset, particle_.anim, 
        (colliderPid_ != entt::null ? std::to_string(static_cast<uint32_t>(colliderPid_)) : "entt::null"));
    return MakeParticleUntied(
        particle_,
        emitterTrans_
    );
}

entt::entity ParticleSystem::MakeParticleTiedColliderImpl(
    const ParticleRecipe &particle_,
    const ComponentTransform &emitterTrans_,
    const entt::entity colliderPid_,
    const ComponentTransform &colliderTrans_,
    const ComponentStaticCollider &colliderCollider_)
{
    auto pid = m_registry.create();

    const auto offset = emitterTrans_.m_pos + particle_.offset - colliderTrans_.m_pos;

    m_registry.emplace<ComponentTransform>(pid, offset, emitterTrans_.m_orientation);

    auto &prim = m_registry.emplace<ComponentParticlePrimitive>(pid);
    prim.lifetime.begin(particle_.lifetime);
    prim.tieTransform = colliderPid_;
    prim.angle = std::atan(colliderCollider_.m_resolved.topAngleCoef()) / std::numbers::pi_v<float> * 180.0f;
    prim.flip = particle_.flip;

    auto &animrnd = m_registry.emplace<ComponentAnimationRenderable>(pid);
    animrnd.loadAnimation(m_animmgmt, particle_.anim);
    animrnd.m_currentAnimation = &animrnd.m_animations.at(particle_.anim);

    m_registry.emplace<RenderLayer>(pid, particle_.layer);

    return pid;
}

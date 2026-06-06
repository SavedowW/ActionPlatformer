#pragma once
#include "ParticleSystem.h"
#include "Core/CoreComponents.h"
#include "Core/ECS/ComponentsView.hpp" // IWYU pragma: keep

template<IsComponentsView ViewT>
entt::entity ParticleSystem::makeParticle(const ParticleRecipe &particle_, const ViewT &view_)
{
    auto pid = m_registry.create();
    const auto &transEmitter = view_.template cget<ComponentTransform>();

    switch (particle_.tiePosRule)
    {
        case TiePosRule::TIE_TO_EMITTER:
        {
            auto offset = particle_.offset;

            m_registry.emplace<ComponentTransform>(pid, offset, transEmitter.m_orientation);

            auto &prim = m_registry.emplace<ComponentParticlePrimitive>(pid);
            prim.lifetime.begin(particle_.lifetime);
            prim.tieTransform = view_.entity();

            auto &animrnd = m_registry.emplace<ComponentAnimationRenderable>(pid);
            animrnd.loadAnimation(m_animmgmt, particle_.anim);
            animrnd.m_currentAnimation = &animrnd.m_animations.at(particle_.anim);

            m_registry.emplace<RenderLayer>(pid, particle_.layer);

            break;
        }

        case TiePosRule::NONE:
        {
            auto offset = particle_.offset;
            if (transEmitter.m_orientation == ORIENTATION::LEFT)
                offset.x *= -1;

            m_registry.emplace<ComponentTransform>(pid, transEmitter.m_pos + offset, transEmitter.m_orientation);

            auto &prim = m_registry.emplace<ComponentParticlePrimitive>(pid);
            prim.lifetime.begin(particle_.lifetime);

            auto &animrnd = m_registry.emplace<ComponentAnimationRenderable>(pid);
            animrnd.loadAnimation(m_animmgmt, particle_.anim);
            animrnd.m_currentAnimation = &animrnd.m_animations.at(particle_.anim);

            m_registry.emplace<RenderLayer>(pid, particle_.layer);

            break;
        }
    }

    return pid;
}


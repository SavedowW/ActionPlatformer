#pragma once
#include "ParticleSystem.h"
#include "Core/CoreComponents.h"
#include "Core/ECS/ComponentsView.hpp" // IWYU pragma: keep

template<IsComponentsView ViewT>
entt::entity ParticleSystem::makeParticle(const ParticleRecipe &particle_, const ViewT &view_)
{
    const auto &transEmitter = view_.template cget<ComponentTransform>();

    switch (particle_.tiePosRule)
    {
        case TiePosRule::TIE_TO_EMITTER:
        {
            return MakeParticleTiedEmitter(
                particle_,
                transEmitter,
                view_.entity()
            );
        }

        case TiePosRule::TIE_TO_GROUND:
        {
            const auto groundId = view_.template cget<WorldPosition>().ground.onGround;
            return MakeParticleTiedCollider(
                particle_,
                transEmitter,
                groundId
            );
        }

        case TiePosRule::TIE_TO_WALL:
        {
            const auto &worldPos = view_.template cget<WorldPosition>();
            const entt::entity wallId = (worldPos.wall.leftWall != entt::null ? worldPos.wall.leftWall : worldPos.wall.rightWall);

            return MakeParticleTiedCollider(
                particle_,
                transEmitter,
                wallId
            );
        }

        case TiePosRule::NONE:
        {
            return MakeParticleUntied(
                particle_,
                transEmitter
            );
        }
    }
}


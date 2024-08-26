#ifndef PARTICLE_SYSTEM_H_
#define PARTICLE_SYSTEM_H_
#include "Application.h"
#include <entt/entt.hpp>

struct ParticleTemplate
{
    ParticleTemplate(int count_, const Vector2<float> &offset_, int anim_, uint32_t lifetime_, size_t layer_,
        utils::Gate<float> &&horizontalFlipGate_, utils::Gate<float> &&verticalFlipGate_);

    ParticleTemplate() = default;
    ParticleTemplate(const ParticleTemplate &rhs_) = default;
    ParticleTemplate(ParticleTemplate &&rhs_) = default;
    ParticleTemplate& operator=(const ParticleTemplate &rhs_) = default;
    ParticleTemplate& operator=(ParticleTemplate &&rhs_) = default;

    int count = 0;
    Vector2<float> offset;
    int anim;
    uint32_t lifetime = 0;
    size_t layer;

    utils::Gate<float> horizontalFlipGate;
    utils::Gate<float> verticalFlipGate;
};

struct ParticleRecipe
{
    ParticleRecipe(const ParticleTemplate &template_);

    int count = 0;
    Vector2<float> pos;
    int anim;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    uint32_t lifetime;
    float angle;
    size_t layer;
};

class ParticleSystem
{
public:
    ParticleSystem(entt::registry &reg_, Application &app_);

    void makeParticle(const ParticleRecipe &particle_);

    template<size_t LAYER>
    void makeParticle(const ParticleRecipe &particle_);

    void update();

private:
    entt::registry &m_registry;
    AnimationManager &m_animmgmt;

};

#endif
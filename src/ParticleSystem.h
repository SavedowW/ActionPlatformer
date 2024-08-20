#ifndef PARTICLE_SYSTEM_H_
#define PARTICLE_SYSTEM_H_
#include "Application.h"
#include <entt/entt.hpp>

struct ParticleTemplate
{
    int count = 0;
    Vector2<float> offset;
    int anim;
    uint32_t lifetime = 0;

    utils::Gate<float> horizontalFlipGate;
    utils::Gate<float> verticalFlipGate;
};

struct ParticleRecipe
{
    int count = 0;
    Vector2<float> pos;
    int anim;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    uint32_t lifetime;
    float angle;
};

class ParticleSystem
{
public:
    ParticleSystem(entt::registry &reg_, Application &app_);
    void makeParticle(const ParticleRecipe &particle_);

    void update();

private:
    entt::registry &m_registry;
    AnimationManager &m_animmgmt;

};

#endif
#ifndef PARTICLE_SYSTEM_H_
#define PARTICLE_SYSTEM_H_
#include "Application.h"
#include <entt/entt.hpp>

enum class TiePosRule
{
    TIE_TO_WALL,
    TIE_TO_GROUND,
    TIE_TO_SOURCE,
    NONE
};

enum class TieLifetimeRule
{
    DESTROY_ON_STATE_LEAVE,
    NONE
};

struct ParticleTemplate
{
    ParticleTemplate(int count_, const Vector2<float> &offset_, int anim_, uint32_t lifetime_, int layer_);

    ParticleTemplate &setTiePosRules(TiePosRule tieRule_);
    ParticleTemplate &setTieLifetimeRules(TieLifetimeRule tieRule_);
    ParticleTemplate &setNotDependOnGroundAngle();

    ParticleTemplate() = default;
    ParticleTemplate(const ParticleTemplate &rhs_) = default;
    ParticleTemplate(ParticleTemplate &&rhs_) = default;
    ParticleTemplate& operator=(const ParticleTemplate &rhs_) = default;
    ParticleTemplate& operator=(ParticleTemplate &&rhs_) = default;

    int count = 0;
    Vector2<float> offset;
    int anim;
    uint32_t lifetime = 0;
    int layer;
    TiePosRule m_tiePosRule = TiePosRule::NONE;
    TieLifetimeRule m_tieLifetimeRule = TieLifetimeRule::NONE;
    bool m_dependOnGroundAngle = true;
};

struct ParticleRecipe
{
    ParticleRecipe(const ParticleTemplate &template_);

    int count = 0;
    Vector2<float> pos;
    int anim;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    uint32_t lifetime;
    float angle = 0;
    size_t layer;
    entt::entity m_tiePosTo = entt::null;

    const ParticleTemplate &m_baseTemplate;
};

class ParticleSystem
{
public:
    ParticleSystem(entt::registry &reg_, Application &app_);

    void makeParticle(const ParticleRecipe &particle_, std::vector<entt::entity> *placeId_ = nullptr);

    void update();

private:
    entt::registry &m_registry;
    AnimationManager &m_animmgmt;

};

#endif
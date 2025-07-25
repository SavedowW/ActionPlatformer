#include "EnvironmentSystem.h"
#include "CoreComponents.h"
#include "EnvComponents.h"

EnvironmentSystem::EnvironmentSystem(Application &app_, entt::registry &reg_) :
    m_app(app_),
    m_reg(reg_)
{
}

void EnvironmentSystem::makeObject(ObjectClass objClass_, const Vector2<int> &pos_, bool visible_, int layer_)
{
    switch (objClass_)
    {
        case ObjectClass::GRASS_TOUCHABLE:
            makeGrassTop(pos_, visible_, layer_);
            break;

        default:
            throw std::string("Trying to create an object of an undefined type");
    }
}

void EnvironmentSystem::makeGrassTop(const Vector2<int> &pos_, bool visible_, int layer_)
{
    auto &animManager = *m_app.getAnimationManager();

    auto objEnt = m_reg.create();
    auto &trans = m_reg.emplace<ComponentTransform>(objEnt, pos_, ORIENTATION::RIGHT);

    auto &animrnd = m_reg.emplace<ComponentAnimationRenderable>(objEnt);
    auto &renLayer = m_reg.emplace<RenderLayer>(objEnt, layer_);
    renLayer.m_visible = visible_;

    animrnd.loadAnimation(animManager, animManager.getAnimID("Environment/grass_single_top"));
    animrnd.loadAnimation(animManager, animManager.getAnimID("Environment/grass_single_top_flickL"), LOOPMETHOD::NOLOOP);
    animrnd.loadAnimation(animManager, animManager.getAnimID("Environment/grass_single_top_flickR"), LOOPMETHOD::NOLOOP);

    auto animSize = animrnd.m_animations.at(animManager.getAnimID("Environment/grass_single_top")).getSize();
    auto animOrigin = animrnd.m_animations.at(animManager.getAnimID("Environment/grass_single_top")).getOrigin();

    animrnd.m_currentAnimation = &animrnd.m_animations.at(animManager.getAnimID("Environment/grass_single_top"));
    animrnd.m_currentAnimation->reset();

    //trans.m_pos.x += (animSize.x);
    //trans.m_pos.y -= (animSize.y);
    trans.m_pos.x += (animOrigin.x - 1);
    trans.m_pos.y -= (animSize.y + 1 - animOrigin.y);

    m_reg.emplace<GrassTopComp>(objEnt);
    GrassTopComp::m_idleAnimId = animManager.getAnimID("Environment/grass_single_top");
    GrassTopComp::m_flickRightAnimId = animManager.getAnimID("Environment/grass_single_top_flickR");
    GrassTopComp::m_flickLeftAnimId = animManager.getAnimID("Environment/grass_single_top_flickL");
}

void EnvironmentSystem::update(entt::entity playerId_)
{
    auto physicals = m_reg.view<ComponentTransform, ComponentPhysical>();
    auto grassTops = m_reg.view<ComponentTransform, GrassTopComp>();

    for (auto [idx, trans, grass] : grassTops.each())
    {
        if (grass.update({&m_reg, idx}))
        {
            for (auto [idx2, trans2, phys2] : physicals.each())
            {
                Collider grassPb;
                if (phys2.m_appliedOffset.x > 0.01f)
                    grassPb = GrassTopComp::m_colliderRight + trans.m_pos;
                else if (phys2.m_appliedOffset.x <= -0.01f)
                    grassPb = GrassTopComp::m_colliderLeft + trans.m_pos;
                else
                    continue;
                    
                auto pb = phys2.m_pushbox + trans2.m_pos;
                auto res = pb.checkOverlap(grassPb);
                if (checkCollision(res, OverlapResult::OVERLAP_BOTH))
                    grass.touchedPlayer(phys2.m_appliedOffset, {&m_reg, idx});
            }
        }
    }
}

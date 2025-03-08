#include "EnvironmentSystem.h"
#include "CoreComponents.h"
#include "EnvComponents.h"

EnvironmentSystem::EnvironmentSystem(Application &app_, entt::registry &reg_) :
    m_app(app_),
    m_reg(reg_)
{
}

void EnvironmentSystem::makeGrassTop(const Vector2<int> &pos_)
{
    auto &animManager = *m_app.getAnimationManager();

    auto objEnt = m_reg.create();
    m_reg.emplace<ComponentTransform>(objEnt, pos_, ORIENTATION::RIGHT);

    auto &animrnd = m_reg.emplace<ComponentAnimationRenderable>(objEnt);
    m_reg.emplace<RenderLayer>(objEnt, 0);

    animrnd.m_animations[animManager.getAnimID("Environment/grass_single_top")] = std::make_unique<Animation>(animManager, animManager.getAnimID("Environment/grass_single_top"), LOOPMETHOD::JUMP_LOOP);
    animrnd.m_animations[animManager.getAnimID("Environment/grass_single_top_flickL")] = std::make_unique<Animation>(animManager, animManager.getAnimID("Environment/grass_single_top_flickL"), LOOPMETHOD::NOLOOP);
    animrnd.m_animations[animManager.getAnimID("Environment/grass_single_top_flickR")] = std::make_unique<Animation>(animManager, animManager.getAnimID("Environment/grass_single_top_flickR"), LOOPMETHOD::NOLOOP);

    animrnd.m_currentAnimation = animrnd.m_animations[animManager.getAnimID("Environment/grass_single_top")].get();
    animrnd.m_currentAnimation->reset();

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

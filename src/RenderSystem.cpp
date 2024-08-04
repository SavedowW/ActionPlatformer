#include "RenderSystem.h"

RenderSystem::RenderSystem(entt::registry &reg_, Application &app_, Camera &camera_) :
    m_reg(reg_),
    m_renderer(*app_.getRenderer()),
    m_camera(camera_)
{
}

void RenderSystem::draw()
{
    auto viewColliders = m_reg.view<ComponentStaticCollider>();
    auto viewTriggers = m_reg.view<ComponentTrigger>();
    auto viewInstances = m_reg.view<ComponentTransform, ComponentAnimationRenderable>();
    auto viewPhysical = m_reg.view<ComponentTransform, ComponentPhysical>();
    auto viewFocuses = m_reg.view<CameraFocusArea>();

    for (auto [idx, scld] : viewColliders.each())
    {
        if (m_reg.all_of<ComponentObstacle>(idx))
            drawObstacle(scld);
        else
            drawCollider(scld);
    }

    for (auto [idx, trg] : viewTriggers.each())
        drawTrigger(trg);

    for (auto [idx, trans, inst] : viewInstances.each())
        drawInstance(trans, inst);

    for (auto [idx, trans, phys] : viewPhysical.each())
        drawCollider(trans, phys);

    for (auto [idx, area] : viewFocuses.each())
        drawFocusArea(area);
}

void RenderSystem::drawInstance(ComponentTransform &trans_, ComponentAnimationRenderable &ren_)
{
    if (ren_.m_currentAnimation != nullptr)
    {
        auto texSize = ren_.m_currentAnimation->getSize();
        auto animorigin = ren_.m_currentAnimation->getOrigin();
        auto texPos = trans_.m_pos;
        texPos.y -= animorigin.y;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (trans_.m_orientation == ORIENTATION::LEFT)
        {
            flip = SDL_FLIP_HORIZONTAL;
            texPos.x -= (texSize.x - animorigin.x);
        }
        else
        {
            texPos.x -= animorigin.x;
        }

        auto spr = ren_.m_currentAnimation->getSprite();
        auto edge = ren_.m_currentAnimation->getBorderSprite();

        m_renderer.renderTexture(spr, texPos.x, texPos.y, texSize.x , texSize.y, m_camera, 0.0f, flip);
    }
}

void RenderSystem::drawCollider(ComponentTransform &trans_, ComponentPhysical &phys_)
{
    if (gamedata::debug::drawColliders)
    {
        auto pb = phys_.m_pushbox + trans_.m_pos;;
        m_renderer.drawCollider(pb, {238, 195, 154, 50}, 100, m_camera);
    }
}

void RenderSystem::drawCollider(ComponentStaticCollider &cld_)
{
    if (gamedata::debug::drawColliders)
    {
        m_renderer.drawCollider(cld_.m_collider, {255, 0, 0, 100}, 255, m_camera);
    }
}

void RenderSystem::drawObstacle(ComponentStaticCollider &cld_)
{
    if (gamedata::debug::drawColliders)
    {
        m_renderer.drawCollider(cld_.m_collider, {50, 50, 255, 100}, 255, m_camera);
    }
}

void RenderSystem::drawTrigger(ComponentTrigger &cld_)
{
    if (gamedata::debug::drawColliders)
    {
        m_renderer.drawCollider(cld_.m_trigger, {255, 50, 255, 50}, 100, m_camera);
    }
}

void RenderSystem::drawFocusArea(CameraFocusArea &cfa_)
{
    cfa_.draw(m_camera);
}

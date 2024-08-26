#include "RenderSystem.h"

RenderSystem::RenderSystem(entt::registry &reg_, Application &app_, Camera &camera_) :
    m_reg(reg_),
    m_renderer(*app_.getRenderer()),
    m_camera(camera_)
{
}

void RenderSystem::update()
{
    auto rens = m_reg.view<ComponentAnimationRenderable>();

    for (auto [idx, ren] : rens.each())
    {
        if (ren.m_currentAnimation)
            ren.m_currentAnimation->update();
    }
}

void RenderSystem::draw()
{
    EXPECTED_RENDER_LAYERS(3);

    const auto viewColliders = m_reg.view<ComponentStaticCollider>();
    const auto viewTriggers = m_reg.view<ComponentTrigger>();
    const auto viewPhysical = m_reg.view<ComponentTransform, ComponentPhysical>();
    auto viewFocuses = m_reg.view<CameraFocusArea>();
    const auto viewTransforms = m_reg.view<ComponentTransform>();

    if constexpr (gamedata::debug::drawColliders)
    {
        for (auto [idx, scld] : viewColliders.each())
        {
            if (scld.m_obstacleId)
                drawObstacle(scld);
            else
                drawCollider(scld);
        }
    }

    if constexpr (gamedata::debug::drawColliders)
    {
        for (auto [idx, trg] : viewTriggers.each())
            drawTrigger(trg);
    }

    handleLayer<0>();
    handleLayer<1>();
    handleLayer<2>();

    if constexpr (gamedata::debug::drawColliders)
    {
        for (auto [idx, trans, phys] : viewPhysical.each())
            drawCollider(trans, phys);
    }

    if constexpr (gamedata::debug::drawFocusAreas)
    {
        for (auto [idx, area] : viewFocuses.each())
            drawFocusArea(area);
    }

    if constexpr (gamedata::debug::drawTransforms)
    {
        for (auto [idx, trans] : viewTransforms.each())
            drawTransform(trans);
    }
}

void RenderSystem::drawInstance(const ComponentTransform &trans_, const ComponentAnimationRenderable &ren_)
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

void RenderSystem::drawParticle(const ComponentTransform &trans_, const ComponentParticlePrimitive &partcl_, const ComponentAnimationRenderable &ren_)
{
    if (ren_.m_currentAnimation != nullptr)
    {
        auto texSize = ren_.m_currentAnimation->getSize();
        auto animorigin = ren_.m_currentAnimation->getOrigin();
        auto texPos = trans_.m_pos;

        if (partcl_.m_flip & SDL_FLIP_HORIZONTAL)
            texPos.x -= (texSize.x - animorigin.x);
        else
            texPos.x -= animorigin.x;
        
        if (partcl_.m_flip & SDL_FLIP_VERTICAL)
            texPos.y -= (texSize.y - animorigin.y);
        else
            texPos.y -= animorigin.y;

        auto spr = ren_.m_currentAnimation->getSprite();
        auto edge = ren_.m_currentAnimation->getBorderSprite();

        m_renderer.renderTexture(spr, texPos.x, texPos.y, texSize.x , texSize.y, m_camera, partcl_.angle, animorigin, partcl_.m_flip);

        if constexpr (gamedata::debug::drawDebugTextures)
        {
            m_renderer.drawRectangle(texPos, texSize, {100, 0, 100, 255}, m_camera);
            m_renderer.fillRectangle(texPos + animorigin - Vector2{2.0f, 2.0f}, {5.0f, 5.0f}, {100, 0, 100, 255}, m_camera);
        }
    }
}

template <size_t LAYER>
void RenderSystem::handleLayer()
{
    const auto viewInstances = m_reg.view<ComponentTransform, ComponentPhysical, ComponentAnimationRenderable, RenderLayer<LAYER>>();
    const auto viewParticles = m_reg.view<ComponentTransform, ComponentParticlePhysics, ComponentParticlePrimitive, ComponentAnimationRenderable, RenderLayer<LAYER>>();

    for (auto [idx, trans, phys, partcl, ren] : viewParticles.each())
        drawParticle(trans, partcl, ren);

    for (auto [idx, trans, phys, inst] : viewInstances.each())
        drawInstance(trans, inst);
}

void RenderSystem::drawCollider(const ComponentTransform &trans_, const ComponentPhysical &phys_)
{
    auto pb = phys_.m_pushbox + trans_.m_pos;
    m_renderer.drawCollider(pb, {238, 195, 154, 50}, {238, 195, 154, 100}, m_camera);

    auto edgex = (trans_.m_orientation == ORIENTATION::RIGHT ? pb.getRightEdge() - 1 : 
                    (trans_.m_orientation == ORIENTATION::LEFT ? pb.getLeftEdge() : pb.m_center.x));

    Vector2 TR{edgex, pb.getTopEdge()};
    Vector2 BR{edgex, pb.getBottomEdge()};
    m_renderer.drawLine(TR, BR, {0, 255, 0, 100}, m_camera);
}

void RenderSystem::drawCollider(const ComponentStaticCollider &cld_)
{
    m_renderer.drawCollider(cld_.m_collider, {255, 0, 0, Uint8(cld_.m_isEnabled ? 100 : 0)}, {255, 0, 0, 255}, m_camera);
}

void RenderSystem::drawObstacle(const ComponentStaticCollider &cld_)
{
    m_renderer.drawCollider(cld_.m_collider, {50, 50, 255, Uint8(cld_.m_isEnabled ? 100 : 0)}, {50, 50, 255, 255}, m_camera);
}

void RenderSystem::drawTrigger(const ComponentTrigger &cld_)
{
    m_renderer.drawCollider(cld_.m_trigger, {255, 50, 255, 50}, {255, 50, 255, 100}, m_camera);
}

void RenderSystem::drawFocusArea(CameraFocusArea &cfa_)
{
    cfa_.draw(m_camera);
}

void RenderSystem::drawTransform(const ComponentTransform &cfa_)
{
    m_renderer.drawLine(cfa_.m_pos - Vector2<float>{0.0f, 5.0f}, cfa_.m_pos + Vector2<float>{0.0f, 5.0f}, {0, 0, 0, 255}, m_camera);
    m_renderer.drawLine(cfa_.m_pos - Vector2<float>{5.0f, 0.0f}, cfa_.m_pos + Vector2<float>{5.0f, 0.0f}, {0, 0, 0, 255}, m_camera);
}

#include "RenderSystem.h"
#include "Core/CoreComponents.h"
#include "Core/Vector2.hpp"
#include "EnvComponents.h"
#include "Core/GameData.h"
#include "Core/Configuration.h"
#include "Core/Application.h"
#include "Core/Logger.hpp"

RenderSystem::RenderSystem(entt::registry &reg_, Camera &camera_, ColliderRoutesCollection &rtCol_) :
    m_reg(reg_),
    m_renderer(Application::instance().m_renderer),
    m_camera(camera_),
    m_routesCollection(rtCol_)
{
    subscribe(GAMEPLAY_EVENTS::REN_DBG_1);
    setInputEnabled();
}

void RenderSystem::update()
{
    auto rens = m_reg.view<ComponentAnimationRenderable>();

    for (auto [idx, ren] : rens.each())
    {
        if (ren.m_currentAnimation && !checkCurrentHitstop(m_reg, idx))
            ren.m_currentAnimation->update();

        if (ren.m_flash)
        {
            if (ren.m_flash->update())
                ren.m_flash.reset();
        }
    }

    auto hprens = m_reg.view<HealthRendererCommonWRT>();
    for (auto [idx, hpren] : hprens.each())
    {
        hpren.update();
    }
}

void RenderSystem::updateDepth()
{
    if (RenderLayer::m_dirtyOrder)
    {
        // FIXME: sometimes with a lot of enemies player can change layer
        std::cout << "Updating depth" << std::endl;
        m_reg.sort<RenderLayer>([](const RenderLayer &lhs_, const RenderLayer &rhs_)
        {
            return lhs_.getDepth() > rhs_.getDepth();
        });
    }

    RenderLayer::m_dirtyOrder = false;
}

void RenderSystem::draw() const
{
    const auto &conf = ConfigurationManager::instance();
    auto viewFocuses = m_reg.view<CameraFocusArea>();
    const auto viewTransforms = m_reg.view<ComponentTransform>();
    const auto viewHealthOwners = m_reg.view<ComponentTransform, HealthRendererCommonWRT>();

    const auto renderable = m_reg.view<RenderLayer, ComponentTransform>();

    for (const auto &[idx, renlayer, trans] : renderable.each())
        if (renlayer.isVisible())
            handleDepthInstance(idx, trans);

    for (const auto &[idx, trans, hren] : viewHealthOwners.each())
        drawHealth(trans, hren);

    if (conf.m_debug.m_drawColliders)
    {
        const auto viewColliders = m_reg.view<ComponentStaticCollider>();
        const auto viewPhysical = m_reg.view<ComponentTransform, ComponentPhysical>();
        const auto viewBtlAct = m_reg.view<ComponentTransform, BattleActor>();
        const auto viewGrassTop = m_reg.view<ComponentTransform, GrassTopComp>();

        for (const auto &[idx, scld] : viewColliders.each())
        {
            if (scld.obstacleType > ObstacleType::NONE)
                drawObstacle(scld);
            else
                drawCollider(scld);
        }
        
        for (const auto &[idx, trans, phys] : viewPhysical.each())
            drawCollider(trans, phys);

        for (const auto &[idx, trans, btl] : viewBtlAct.each())
            drawBattleActorColliders(trans, btl);

        for (const auto &[idx, trans, grass] : viewGrassTop.each())
        {
            const auto pbl = GrassTopComp::colliderLeft + trans.m_pos;
            const auto pbr = GrassTopComp::colliderRight + trans.m_pos;
            m_renderer.drawCollider(pbl, {238, 195, 154, 50}, m_camera);
            m_renderer.drawCollider(pbr, {238, 195, 154, 50}, m_camera);
        }
    }

    if (conf.m_debug.m_drawFocusAreas)
    {
        for (const auto& [idx, area] : viewFocuses.each())
            drawFocusArea(area);
    }

    if (conf.m_debug.m_drawTransforms)
    {
        for (const auto &[idx, trans] : viewTransforms.each())
            drawTransform(trans);
    }

    if (conf.m_debug.m_drawColliderRoutes)
    {
        for (const auto &el : m_routesCollection)
            drawColliderRoute(el.second);
    }
}

void RenderSystem::drawInstance(const ComponentTransform &trans_, const ComponentAnimationRenderable &ren_) const
{
    if (ren_.m_currentAnimation != nullptr)
    {
        auto texSize = ren_.m_currentAnimation->getSize();
        auto animorigin = ren_.m_currentAnimation->getOrigin();
        auto texPos = trans_.m_pos + Vector2{1, 1};
        texPos.y -= animorigin.y;
        SDL_FlipMode flip = SDL_FLIP_NONE;
        if (trans_.m_orientation == Orientation::LEFT)
        {
            flip = SDL_FLIP_HORIZONTAL;
            texPos.x -= (texSize.x - animorigin.x);
        }
        else
        {
            texPos.x -= animorigin.x;
        }

        auto spr = ren_.m_currentAnimation->getSprite();

        if (ren_.m_drawOutline)
            m_renderer.renderTextureOutlined(spr, texPos, texSize, flip, m_camera);
        else
            m_renderer.renderTexture(spr, texPos, texSize, flip, 1.0f, m_camera);

        if (ren_.m_flash)
        {
            auto alpha = ren_.m_flash->getFlashAlpha();
            //std::cout << (int)alpha << std::endl;
            m_renderer.renderTextureFlash(spr, texPos, texSize, flip, alpha, m_camera);
        }

        if (ConfigurationManager::instance().m_debug.m_drawDebugTextures)
        {
            m_renderer.drawRectangle(texPos, texSize, {100, 0, 100, 255}, m_camera);
            m_renderer.fillRectangle(texPos + animorigin - Vector2{2, 2}, {5, 5}, {100, 0, 100, 255}, m_camera);
        }
    }
}

void RenderSystem::drawParticle(const ComponentTransform &trans_, const ComponentParticlePrimitive &partcl_, const ComponentAnimationRenderable &ren_) const
{
    if (ren_.m_currentAnimation != nullptr)
    {
        const auto texSize = ren_.m_currentAnimation->getSize();
        const auto animorigin = ren_.m_currentAnimation->getOrigin();
        auto texPos = trans_.m_pos;
        if (partcl_.tieTransform != entt::null)
        {
            const auto &tiedTrans = m_reg.get<ComponentTransform>(partcl_.tieTransform);
            if (tiedTrans.m_orientation == Orientation::LEFT)
                texPos = tiedTrans.m_pos.add(-texPos.x, texPos.y);
            else
                texPos = tiedTrans.m_pos.add(texPos.x, texPos.y);
        }

        SDL_FlipMode flip = SDL_FLIP_NONE;

        if (trans_.m_orientation == Orientation::LEFT)
        {
            texPos.x -= (texSize.x - animorigin.x) - 2;
            flip = SDL_FLIP_HORIZONTAL;
        }
        else
            texPos.x -= animorigin.x;

        texPos.y -= animorigin.y;

        const auto spr = ren_.m_currentAnimation->getSprite();

        m_renderer.renderTexture(spr, texPos, texSize, flip, partcl_.angle, animorigin, m_camera);

        if (ConfigurationManager::instance().m_debug.m_drawDebugTextures)
        {
            m_renderer.drawRectangle(texPos, texSize, {100, 0, 100, 255}, m_camera);
            m_renderer.fillRectangle(texPos + animorigin - Vector2{2, 2}, {5, 5}, {100, 0, 100, 255}, m_camera);
        }
    }
}

void RenderSystem::drawTilemapLayer(const ComponentTransform &trans_, const TilemapLayer &tilemap_) const
{
    const Vector2<int> camTL = Vector2<int>(m_camera.getPos().mulComponents(tilemap_.m_parallaxFactor)) - Vector2<int>(gamedata::global::maxCameraSize) / 2;

    Vector2<int> dstPos;
    dstPos.x = trans_.m_pos.x + tilemap_.m_posOffset.x - camTL.x;
    dstPos.y = trans_.m_pos.y + tilemap_.m_posOffset.y - camTL.y;

    for (const auto &row : tilemap_.m_tiles)
    {
        dstPos.x = trans_.m_pos.x + tilemap_.m_posOffset.x - camTL.x;
        for (const auto &tile : row)
        {
            if (tile.m_tile)
            {
                m_renderer.renderTile(tile.m_tile->m_tex, dstPos, gamedata::tiles::tileSize, tile.m_flip, tile.m_tile->m_tilePos);
            }
            dstPos.x += gamedata::tiles::tileSize.x;
        }
        dstPos.y += gamedata::tiles::tileSize.y;
    }
}

void RenderSystem::handleDepthInstance(const entt::entity &idx_, const ComponentTransform &trans_) const
{
    if (auto *ren = m_reg.try_get<ComponentAnimationRenderable>(idx_))
    {
        if (auto *partcl = m_reg.try_get<ComponentParticlePrimitive>(idx_))
        {
            // Definitely particle
            drawParticle(trans_, *partcl, *ren);
        }
        else
        {
            // Definitely instance
            drawInstance(trans_, *ren);
        }
    }
    else if (auto *tilemap = m_reg.try_get<TilemapLayer>(idx_))
    {
        drawTilemapLayer(trans_, *tilemap);
    }
}

void RenderSystem::drawBattleActorColliders(const ComponentTransform &trans_, const BattleActor &btlact_) const
{
    if (btlact_.m_hurtboxes)
    {
        for (const auto &group : *(btlact_.m_hurtboxes))
        {
            for (const auto &tcld : group.m_colliders)
            {
                if (tcld.m_timeline[btlact_.m_currentFrame])
                {
                    m_renderer.drawCollider(getColliderAt(tcld.m_collider, trans_), gamedata::characters::hurtboxColor, m_camera);
                }
            }
        }
    }

    for (const auto *hit : btlact_.m_activeHits)
    {
        for (const auto &tmpcld : hit->m_colliders)
        {
            if (tmpcld.m_timeline[btlact_.m_currentFrame])
                m_renderer.drawCollider(getColliderAt(tmpcld.m_collider, trans_), gamedata::characters::hitboxColor, m_camera);
        }
    }
}

void RenderSystem::drawCollider(const ComponentTransform &trans_, const ComponentPhysical &phys_) const
{
    auto pb = phys_.pushbox + trans_.m_pos;
    m_renderer.drawCollider(pb, gamedata::characters::pushboxColor, m_camera);

    auto edgex = (trans_.m_orientation == Orientation::RIGHT ? pb.getRightEdge() + 1 : 
                    (trans_.m_orientation == Orientation::LEFT ? pb.getLeftEdge() + 1 : pb.m_topLeft.x + pb.m_size.x / 2));

    Vector2 TR{edgex, pb.getTopEdge() + 6};
    Vector2 BR{edgex, pb.getBottomEdge() + 1};
    m_renderer.drawLine(TR, BR, {0, 255, 0, 100}, m_camera);
}

void RenderSystem::drawCollider(const ComponentStaticCollider &cld_) const
{
    if (cld_.m_isEnabled)
        m_renderer.drawCollider(cld_.m_resolved, {255, 0, 0, 100}, m_camera);
}

void RenderSystem::drawObstacle(const ComponentStaticCollider &cld_) const
{
    if (cld_.m_isEnabled)
        m_renderer.drawCollider(cld_.m_resolved, {50, 50, 255, 100}, m_camera);
}

void RenderSystem::drawFocusArea(const CameraFocusArea &cfa_) const
{
    cfa_.draw(m_camera);
}

void RenderSystem::drawTransform(const ComponentTransform &trans_) const
{
    m_renderer.drawCross(trans_.m_pos, {1, 10}, {10, 1}, {0, 0, 0, 255}, m_camera);
}

void RenderSystem::drawHealth(const ComponentTransform &trans_, const  HealthRendererCommonWRT &howner_) const
{
    if (howner_.m_state == HealthRendererCommonWRT::DelayFadeStates::INACTIVE)
        return;

    const auto worldPos = trans_.m_pos + howner_.m_offset;

    if (ConfigurationManager::instance().m_debug.m_drawHealthPos)
        m_renderer.drawCross(worldPos, {1, 5}, {5, 1}, {255, 0, 0, 255}, m_camera);

    assert(!howner_.m_heartAnims.empty());

    const auto texSize = howner_.m_heartAnims[0].getSize();
    const auto animorigin = howner_.m_heartAnims[0].getOrigin();

    const auto texCenter = (worldPos - animorigin).sub(0, texSize.y - 20);

    const float mid = (float)(howner_.m_heartAnims.size() - 1) / 2.0f;
    float cnt = 0;

    for (const auto &el : howner_.m_heartAnims)
    {
        // TODO: make proper "over" check
        if (!el.getDirection())
            continue;

        const auto offsetMul = cnt - mid;
        auto spr = el.getSprite();
        auto texPos = texCenter;
        texPos.x += (texSize.x - 19) * offsetMul;

        float alpha = 1.0f;
        
        if (howner_.m_state == HealthRendererCommonWRT::DelayFadeStates::FADE_IN)
            alpha = howner_.m_delayFadeTimer.getProgressNormalized();
        else if (howner_.m_state == HealthRendererCommonWRT::DelayFadeStates::FADE_OUT)
            alpha = 1.0f - howner_.m_delayFadeTimer.getProgressNormalized();

        m_renderer.renderTexture(spr, texPos, texSize, SDL_FLIP_NONE, alpha, m_camera);

        //m_renderer.drawRectangle(texPos, texSize, {0, 0, 0, 255}, m_camera);

        cnt++;
    }
}

void RenderSystem::drawColliderRoute(const ColliderPointRouting &route_) const
{
    const Vector2<float> nodeSize{5.0f, 5.0f};

    if (route_.m_dbgIter == 0)
    {
        m_renderer.fillRectangle(route_.m_origin.m_pos - nodeSize / 2.0f, nodeSize, {255, 127, 39, 255}, m_camera);
    }
    else
    {
        auto newPoint = route_.m_links[route_.m_dbgIter - 1].m_target.m_pos;
        auto prevPoint = route_.m_origin.m_pos;
        if (route_.m_dbgIter > 1)
            prevPoint = route_.m_links[route_.m_dbgIter - 2].m_target.m_pos;

        m_renderer.fillRectangle(newPoint - nodeSize / 2.0f, nodeSize, {0, 255, 0, 100}, m_camera);
        m_renderer.drawLine(prevPoint - Vector2{1.0f, 1.0f}, newPoint - Vector2{1.0f, 1.0f}, {0, 255, 0, 100}, m_camera);
    }
}

void RenderSystem::receiveEvents(GAMEPLAY_EVENTS event_, const float scale_)
{
    switch (event_)
    {
        case GAMEPLAY_EVENTS::REN_DBG_1:
            if (scale_ > 0)
                for (auto &el : m_routesCollection)
                    el.second.m_dbgIter = (el.second.m_dbgIter + 1 > el.second.m_links.size() ? 0 : el.second.m_dbgIter + 1);
            break;

        default:
            std::cout << "RenderSystem received undefined event " << int(event_) << std::endl;
    }
}

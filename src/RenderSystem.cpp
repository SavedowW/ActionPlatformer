#include "RenderSystem.h"
#include "Profile.h"

RenderSystem::RenderSystem(entt::registry &reg_, Application &app_, Camera &camera_) :
    m_reg(reg_),
    m_renderer(*app_.getRenderer()),
    m_camera(camera_)
{
}

void RenderSystem::update()
{
    PROFILE_FUNCTION;
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
    PROFILE_FUNCTION;

    if (RenderLayer::m_dirtyOrder)
    {
        std::cout << "Updating depth" << std::endl;
        m_reg.sort<RenderLayer>([](const RenderLayer &lhs_, const RenderLayer &rhs_)
        {
            return lhs_.m_depth > rhs_.m_depth;
        });
    }

    RenderLayer::m_dirtyOrder = false;
}

void RenderSystem::draw()
{
    const auto viewColliders = m_reg.view<ComponentStaticCollider>();
    const auto viewTriggers = m_reg.view<ComponentTrigger>();
    const auto viewPhysical = m_reg.view<ComponentTransform, ComponentPhysical>();
    auto viewFocuses = m_reg.view<CameraFocusArea>();
    const auto viewTransforms = m_reg.view<ComponentTransform>();
    const auto viewBtlAct = m_reg.view<ComponentTransform, BattleActor>();
    const auto viewHealthOwners = m_reg.view<ComponentTransform, HealthRendererCommonWRT>();

    const auto renderable = m_reg.view<RenderLayer>();

    for (auto [idx, renlayer] : renderable.each())
        handleDepthInstance(idx, renlayer);

    for (auto [idx, trans, hren] : viewHealthOwners.each())
        drawHealth(trans, hren);

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

    if constexpr (gamedata::debug::drawColliders)
    {
        for (auto [idx, trans, phys] : viewPhysical.each())
            drawCollider(trans, phys);

        for (auto [idx, trans, btl] : viewBtlAct.each())
            drawBattleActorColliders(trans, btl);
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

        if (ren_.m_flash)
        {
            auto col = ren_.m_flash->getFlashColor();
            auto whitespr = ren_.m_currentAnimation->getWhiteSprite();
            SDL_SetTextureColorMod(whitespr, col.r, col.g, col.b);
            SDL_SetTextureAlphaMod(whitespr, col.a);
            m_renderer.renderTexture(whitespr, texPos.x, texPos.y, texSize.x , texSize.y, m_camera, 0.0f, flip);
        }

        if constexpr (gamedata::debug::drawDebugTextures)
        {
            m_renderer.drawRectangle(texPos, texSize, {100, 0, 100, 255}, m_camera);
            m_renderer.fillRectangle(texPos + animorigin - Vector2{2.0f, 2.0f}, {5.0f, 5.0f}, {100, 0, 100, 255}, m_camera);
        }
    }
}

void RenderSystem::drawParticle(const ComponentTransform &trans_, const ComponentParticlePrimitive &partcl_, const ComponentAnimationRenderable &ren_)
{
    if (ren_.m_currentAnimation != nullptr)
    {
        auto texSize = ren_.m_currentAnimation->getSize();
        auto animorigin = ren_.m_currentAnimation->getOrigin();
        auto texPos = trans_.m_pos;
        if (partcl_.m_tieTransform != entt::null)
            texPos += m_reg.get<ComponentTransform>(partcl_.m_tieTransform).m_pos;

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

void RenderSystem::drawTilemapLayer(const ComponentTransform &trans_, TilemapLayer &tilemap_)
{
    auto old = m_renderer.setRenderTarget(tilemap_.m_tex.getSprite());
    m_renderer.fillRenderer({255, 255, 255, 0});

    Vector2<int> camTL = Vector2<int>(m_camera.getPos().mulComponents(tilemap_.m_parallaxFactor)) - Vector2<int>(gamedata::global::maxCameraSize) / 2;

    SDL_Rect dst;
    dst.w = gamedata::global::tileSize.x;
    dst.h = gamedata::global::tileSize.y;
    dst.x = trans_.m_pos.x -camTL.x;
    dst.y = trans_.m_pos.y -camTL.y;

    for (auto &row : tilemap_.m_tiles)
    {
        dst.x = trans_.m_pos.x - camTL.x;
        for (auto &tile : row)
        {
            if (tile.m_tile)
            {
                m_renderer.renderTexture(tile.m_tile->m_tex, &tile.m_tile->m_src, &dst, 0, nullptr, tile.m_flip);
            }
            dst.x += gamedata::global::tileSize.x;
        }
        dst.y += gamedata::global::tileSize.y;
    }
    m_renderer.setRenderTarget(old);
    m_renderer.renderTexture(tilemap_.m_tex.getSprite(), 0, 0, tilemap_.m_tex.m_w, tilemap_.m_tex.m_h);
}

void RenderSystem::handleDepthInstance(const entt::entity &idx_, const RenderLayer &layer_)
{
    if (auto *ren = m_reg.try_get<ComponentAnimationRenderable>(idx_))
    {
        if (auto *partcl = m_reg.try_get<ComponentParticlePrimitive>(idx_))
        {
            // Definitely particle
            drawParticle(m_reg.get<ComponentTransform>(idx_), *partcl, *ren);
        }
        else
        {
            // Definitely instance
            drawInstance(m_reg.get<ComponentTransform>(idx_), *ren);
        }
    }
    else if (auto *tilemap = m_reg.try_get<TilemapLayer>(idx_))
    {
        drawTilemapLayer(m_reg.get<ComponentTransform>(idx_), *tilemap);
    }
}

void RenderSystem::drawBattleActorColliders(const ComponentTransform &trans_, const BattleActor &btlact_)
{
    if (btlact_.m_hurtboxes)
    {
        for (const auto &group : *(btlact_.m_hurtboxes))
        {
            for (const auto &tcld : group.m_colliders)
            {
                if (tcld.m_timeline[btlact_.m_currentFrame])
                {
                    m_renderer.drawCollider(getColliderAt(tcld.m_collider, trans_), gamedata::characters::hurtboxColor, gamedata::characters::hurtboxColor, m_camera);
                }
            }
        }
    }

    for (const auto *hit : btlact_.m_activeHits)
    {
        for (const auto &tmpcld : hit->m_colliders)
        {
            if (tmpcld.m_timeline[btlact_.m_currentFrame])
                m_renderer.drawCollider(getColliderAt(tmpcld.m_collider, trans_), gamedata::characters::hitboxColor, gamedata::characters::hitboxColor, m_camera);
        }
    }
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
    m_renderer.drawCollider(cld_.m_resolved, {255, 0, 0, Uint8(cld_.m_isEnabled ? 100 : 0)}, {255, 0, 0, 255}, m_camera);
}

void RenderSystem::drawObstacle(const ComponentStaticCollider &cld_)
{
    m_renderer.drawCollider(cld_.m_resolved, {50, 50, 255, Uint8(cld_.m_isEnabled ? 100 : 0)}, {50, 50, 255, 255}, m_camera);
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
    m_renderer.drawCross(cfa_.m_pos, {1.0f, 10.0f}, {10.0f, 1.0f}, {0, 0, 0, 255}, m_camera);
}

void RenderSystem::drawHealth(const ComponentTransform &trans_, const HealthRendererCommonWRT &howner_)
{
    if (howner_.m_state == HealthRendererCommonWRT::DelayFadeStates::INACTIVE)
        return;

    Vector2<float> worldPos = trans_.m_pos + howner_.m_offset;

    if (gamedata::debug::drawHealthPos)
        m_renderer.drawCross(worldPos, {1.0f, 5.0f}, {5.0f, 1.0f}, {255, 0, 0, 255}, m_camera);

    if (!howner_.m_heartAnims.empty() && howner_.m_heartAnims[0])
    {
        auto texSize = howner_.m_heartAnims[0]->getSize();
        auto animorigin = howner_.m_heartAnims[0]->getOrigin();

        auto texCenter = worldPos - animorigin;
        texCenter.y -= (texSize.y - 20);

        float mid = (float)(howner_.m_heartAnims.size() - 1) / 2.0f;
        int cnt = 0;

        for (auto &el : howner_.m_heartAnims)
        {
            if (!el)
                continue;

            auto offsetMul = cnt - mid;
            auto spr = el->getSprite();
            auto texPos = texCenter;
            texPos.x += (texSize.x - 19) * offsetMul;

            uint8_t alpha = 255;
            
            if (howner_.m_state == HealthRendererCommonWRT::DelayFadeStates::FADE_IN)
                alpha *= howner_.m_delayFadeTimer.getProgressNormalized();
            else if (howner_.m_state == HealthRendererCommonWRT::DelayFadeStates::FADE_OUT)
                alpha *= 1 - howner_.m_delayFadeTimer.getProgressNormalized();

            if (alpha != 255)
            {
                uint8_t oldAlpha = 0;
                SDL_GetTextureAlphaMod(spr, &oldAlpha);
                SDL_SetTextureAlphaMod(spr, alpha);
                m_renderer.renderTexture(spr, texPos.x, texPos.y, texSize.x , texSize.y, m_camera, 0.0f, SDL_FLIP_NONE);
                SDL_SetTextureAlphaMod(spr, oldAlpha);
            }
            else
                m_renderer.renderTexture(spr, texPos.x, texPos.y, texSize.x , texSize.y, m_camera, 0.0f, SDL_FLIP_NONE);

            //m_renderer.drawRectangle(texPos, texSize, {0, 0, 0, 255}, m_camera);

            cnt++;
        }
    }
}

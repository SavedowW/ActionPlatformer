#ifndef RENDER_SYSTEM_H_
#define RENDER_SYSTEM_H_
#include "Hit.h"
#include "Core/InputSystem.h"
#include "Core/CoreComponents.h"
#include "Core/CameraFocusArea.h"
#include "ColliderRouting.h"
#include <entt/entt.hpp>

struct RenderSystem : public InputReactor
{
    RenderSystem(entt::registry &reg_, Camera &camera_, ColliderRoutesCollection &rtCol_);

    void update();
    void updateDepth();
    void draw() const;

    void drawInstance(const ComponentTransform &trans_, const ComponentAnimationRenderable &ren_) const;
    void drawParticle(const ComponentTransform &trans_, const ComponentParticlePrimitive &partcl_, const ComponentAnimationRenderable &ren_) const;
    void drawTilemapLayer(const ComponentTransform &trans_, const TilemapLayer &tilemap_) const;

    void handleDepthInstance(const entt::entity &idx_) const;


    void drawBattleActorColliders(const ComponentTransform &trans_, const BattleActor &btlact_) const;
    void drawCollider(const ComponentTransform &trans_, const ComponentPhysical &phys_) const;
    void drawCollider(const ComponentStaticCollider &cld_) const;
    void drawObstacle(const ComponentStaticCollider &cld_) const;
    void drawTrigger(const ComponentTrigger &cld_) const;
    void drawFocusArea(const CameraFocusArea &cfa_) const;
    void drawTransform(const ComponentTransform &trans_) const;
    void drawHealth(const ComponentTransform &trans_, const HealthRendererCommonWRT &howner_) const;

    void drawColliderRoute(const ColliderPointRouting &route_) const;

    void receiveEvents(GAMEPLAY_EVENTS event_, float scale_) override;

    entt::registry &m_reg;
    Renderer &m_renderer;
    Camera &m_camera;
    ColliderRoutesCollection &m_routesCollection;
};

#endif
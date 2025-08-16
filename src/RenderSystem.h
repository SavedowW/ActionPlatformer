#ifndef RENDER_SYSTEM_H_
#define RENDER_SYSTEM_H_
#include "Hit.h"
#include "InputSystem.h"
#include "CoreComponents.h"
#include "CameraFocusArea.h"
#include "ColliderRouting.h"
#include <entt/entt.hpp>

struct RenderSystem : public InputReactor
{
    RenderSystem(entt::registry &reg_, Camera &camera_, ColliderRoutesCollection &rtCol_);

    void update();
    void updateDepth();
    void draw();

    void drawInstance(const ComponentTransform &trans_, const ComponentAnimationRenderable &ren_);
    void drawParticle(const ComponentTransform &trans_, const ComponentParticlePrimitive &partcl_, const ComponentAnimationRenderable &ren_);
    void drawTilemapLayer(const ComponentTransform &trans_, TilemapLayer &tilemap_);

    void handleDepthInstance(const entt::entity &idx_);


    void drawBattleActorColliders(const ComponentTransform &trans_, const BattleActor &btlact_);
    void drawCollider(const ComponentTransform &trans_, const ComponentPhysical &phys_);
    void drawCollider(const ComponentStaticCollider &cld_);
    void drawObstacle(const ComponentStaticCollider &cld_);
    void drawTrigger(const ComponentTrigger &cld_);
    void drawFocusArea(CameraFocusArea &cfa_);
    void drawTransform(const ComponentTransform &trans_);
    void drawHealth(const ComponentTransform &trans_, HealthRendererCommonWRT &howner_);

    void drawColliderRoute(const ColliderPointRouting &route_);

    void receiveEvents(GAMEPLAY_EVENTS event_, const float scale_) override;

    entt::registry &m_reg;
    Renderer &m_renderer;
    Camera &m_camera;
    ColliderRoutesCollection &m_routesCollection;
};

#endif
#ifndef RENDER_SYSTEM_H_
#define RENDER_SYSTEM_H_
#include "CoreComponents.h"
#include "Application.h"
#include "CameraFocusArea.h"
#include <entt/entt.hpp>

struct RenderSystem
{
    RenderSystem(entt::registry &reg_, Application &app_, Camera &camera_);

    void draw();

    void drawInstance(ComponentTransform &trans_, ComponentAnimationRenderable &ren_);
    void drawCollider(ComponentTransform &trans_, ComponentPhysical &phys_);
    void drawCollider(ComponentStaticCollider &cld_);
    void drawObstacle(ComponentStaticCollider &cld_);
    void drawTrigger(ComponentTrigger &cld_);
    void drawFocusArea(CameraFocusArea &cfa_);


    entt::registry &m_reg;
    Renderer &m_renderer;
    Camera &m_camera;
};

#endif
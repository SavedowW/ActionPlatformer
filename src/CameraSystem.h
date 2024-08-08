#ifndef CAMERA_SYSTEM_H_
#define CAMERA_SYSTEM_H_
#include "Camera.h"
#include "FrameTimer.h"
#include "Renderer.h"
#include "Camera.h"
#include <entt/entt.hpp>
#include <optional>

struct CameraSystem
{
    CameraSystem(entt::registry &reg_, Camera &cam_);

    void update();
    bool updateFocus(const Collider &playerPb_);
    void debugDraw(Renderer &ren_, Camera &cam_);

    entt::registry &m_reg;
    entt::entity m_playerId;
    Camera &m_cam;
    std::optional<entt::entity> m_currentFocusArea;

    FrameTimer<false> m_hResetDelay;
    FrameTimer<false> m_vResetDelay;

    const uint32_t H_DELAY = 60;
    const uint32_t V_DELAY = 60;

    const float H_RESET_OFFSET = 2.0f;
    const float V_RESET_OFFSET = 2.0f;

    const Vector2<float> BODY_OFFSET{0.0f, -30.0f};
};

#endif

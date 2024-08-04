#ifndef CAMERA_SYSTEM_H_
#define CAMERA_SYSTEM_H_
#include "Camera.h"
#include <entt/entt.hpp>
#include <optional>

struct CameraSystem
{
    CameraSystem(entt::registry &reg_, Camera &cam_);

    void update();
    bool updateFocus(const Collider &playerPb_);

    entt::registry &m_reg;
    entt::entity m_playerId;
    Camera &m_cam;
    std::optional<entt::entity> m_currentFocusArea;
};

#endif

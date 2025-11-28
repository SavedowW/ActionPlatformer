#ifndef CAMERA_SYSTEM_H_
#define CAMERA_SYSTEM_H_
#include "Core/Camera.h"
#include "Core/ManualTimer.h"
#include "Core/Renderer.h"
#include "Core/InputSystem.h"
#include "Core/Timer.h"
#include <entt/entt.hpp>
#include <optional>

struct CameraSystem : public InputReactor
{
    CameraSystem(entt::registry &reg_, Camera &cam_);

    void update();
    bool updateFocus(const Collider &playerPb_);
    void debugDraw(Renderer &ren_, const Camera &cam_) const;

    void receiveEvents(GAMEPLAY_EVENTS event, float scale_) override;

    entt::registry &m_reg;
    entt::entity playerId;
    Camera &m_cam;
    std::optional<entt::entity> m_currentFocusArea;

    bool m_cameraStopped = false;

    ManualTimer<false> m_hResetDelay;
    ManualTimer<false> m_vResetDelay;

    const Time::NS H_DELAY = Time::fromFrames(60);
    const Time::NS V_DELAY = Time::fromFrames(60);

    const int H_RESET_OFFSET = 2;
    const int V_RESET_OFFSET = 2;

    const Vector2<int> BODY_OFFSET{0, -30};
};

#endif

#include "CameraSystem.h"
#include "CameraFocusArea.h"
#include "CoreComponents.h"

CameraSystem::CameraSystem(entt::registry &reg_, Camera &cam_) :
    m_reg(reg_),
    m_cam(cam_)
{
}

void CameraSystem::update()
{
    Vector2<float> target;
    auto [trans, phys, dtar] = m_reg.get<ComponentTransform, ComponentPhysical, ComponentDynamicCameraTarget>(m_playerId);
    if ((phys.m_velocity + phys.m_inertia) != Vector2{0.0f, 0.0f})
    {
        auto tarcamoffset = utils::limitVectorLength((phys.m_velocity + phys.m_inertia).mulComponents(Vector2{1.0f, 0.0f}) * 30, 100.0f);
        auto deltaVec = tarcamoffset - dtar.m_offset;
        auto dlen = deltaVec.getLen();
        auto ddir = deltaVec.normalised();
        float offsetLen = pow(dlen, 2.0f) / 400.0f;
        offsetLen = utils::clamp(offsetLen, 0.0f, dlen);

        dtar.m_offset += ddir * offsetLen;
    }

    target = trans.m_pos - Vector2{0.0f, 60.0f} + dtar.m_offset;

    if (updateFocus(phys.m_pushbox + trans.m_pos))
    {
        auto &area = m_reg.get<CameraFocusArea>(*m_currentFocusArea);
        m_cam.smoothMoveTowards(area.getCameraTargetPosition(target), {1.0f, 1.0f}, 0, 1.3f, 20.0f);
        m_cam.smoothScaleTowards(area.getScale());
    }
    else
    {
        m_cam.smoothMoveTowards(target, {1.0f, 0.5f}, 5.0f, 1.6f, 80.0f);
        m_cam.smoothScaleTowards(gamedata::global::maxCameraScale);
    }
}

bool CameraSystem::updateFocus(const Collider &playerPb_)
{
    if (m_currentFocusArea)
    {
        auto &area = m_reg.get<CameraFocusArea>(*m_currentFocusArea);
        if (area.checkIfEnters(playerPb_, true))
            return true;
        else
            m_currentFocusArea.reset();
    }

    auto areas = m_reg.view<CameraFocusArea>();

    for (auto [idx, area]: areas.each())
    {
        if (area.checkIfEnters(playerPb_, false))
        {
            m_currentFocusArea = idx;
            return true;
        }
    }

    return false;
}

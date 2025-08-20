#include "CameraSystem.h"
#include "Core/CameraFocusArea.h"
#include "Core/CoreComponents.h"
#include "Core/GameData.h"
#include "Core/Profile.h"
#include "Core/Configuration.h"

CameraSystem::CameraSystem(entt::registry &reg_, Camera &cam_) :
    m_reg(reg_),
    m_cam(cam_)
{
    subscribe(GAMEPLAY_EVENTS::CAM_STOP);
    setInputEnabled();
}

void CameraSystem::update()
{
    PROFILE_FUNCTION;

    if (m_cameraStopped)
        return;

    Vector2<int> target;
    auto [trans, phys, dtar] = m_reg.get<ComponentTransform, ComponentPhysical, ComponentDynamicCameraTarget>(m_playerId);

    if (phys.m_appliedOffset.x != 0)
    {
        auto targetoffset = utils::signof(phys.m_appliedOffset.x) * std::min(abs(phys.m_appliedOffset.x * 30), 100);
        auto delta = (targetoffset - dtar.m_offset.x) * dtar.m_lookaheadSpeedSensitivity.x;
        int realOffset = 0;
        if (abs(delta) <= 1.0f)
        {
            realOffset = static_cast<int>(delta);
        }
        else
        {
            realOffset = static_cast<int>(utils::signof(delta) * pow(abs(delta), 2.0f) / 400.0f);
        }
        dtar.m_offset.x += realOffset;

        m_hResetDelay.begin(H_DELAY);
    }
    else
    {
        if (m_hResetDelay.update())
        {
            dtar.m_offset.x -= utils::signof(dtar.m_offset.x) * utils::clamp(H_RESET_OFFSET, 0, abs(dtar.m_offset.x));
        }
    }

    if (phys.m_appliedOffset.y != 0)
    {
        float vprio = (phys.m_onSlopeWithAngle == 0.0f && phys.m_onMovingPlatform == false ? 0.0f : 1.5f);
        if (phys.m_appliedOffset.y >= 5.0f)
            vprio = 3.0f;
        else if (phys.m_onSlopeWithAngle != 0.0f)
            vprio = 1.5f;
        auto targetoffset = utils::signof(phys.m_appliedOffset.y) * std::min(abs(phys.m_appliedOffset.y * vprio * 20.0f), 40.0f);
        auto delta = (targetoffset - dtar.m_offset.y) * dtar.m_lookaheadSpeedSensitivity.y;
        int realOffset = 0;
        if (abs(delta) <= 1.0f)
        {
            realOffset = static_cast<int>(delta);
        }
        else
        {
            realOffset = static_cast<int>(utils::signof(delta) * pow(abs(delta), 2.0f) / 400.0f);
        }
        dtar.m_offset.y += realOffset;

        m_vResetDelay.begin(V_DELAY);
    }
    else
    {
        if (m_vResetDelay.update())
        {
            dtar.m_offset.y -= utils::signof(dtar.m_offset.y) * utils::clamp(V_RESET_OFFSET, 0, abs(dtar.m_offset.y));
        }
    }

    if (phys.m_onSlopeWithAngle == 0.0f)
        dtar.m_offset.y = utils::signof(dtar.m_offset.y) * utils::clamp(abs(dtar.m_offset.y), 0, 20);

    target = Vector2<int>{trans.m_pos} + BODY_OFFSET + Vector2<int>{dtar.m_offset};
    //std::cout << "dtar.m_offset: " << dtar.m_offset << std::endl;

    if (updateFocus(phys.m_pushbox + trans.m_pos))
    {
        auto &area = m_reg.get<CameraFocusArea>(*m_currentFocusArea);
        m_cam.smoothMoveAxisTowards(area.getCameraTargetPosition(target), {1.0f, 1.0f}, {0.0f, 0.0f}, {10.0f, 10.0f});
        m_cam.smoothScaleTowards(area.getScale());
    }
    else
    {
        const Vector2<float> rangesForMax{10.0f, 10.0f};
        const Vector2<float> rangesForMin{200.0f, 100.0f};
        auto dst = target - m_cam.getPos();

        const auto ranges = rangesForMin - rangesForMax;
        const auto dstRanged = dst.abs() - rangesForMax;
        const Vector2<float> alphas{1 - utils::clamp(dstRanged.x / ranges.x, 0.0f, 1.0f), 1 - utils::clamp(dstRanged.y / ranges.y, 0.0f, 1.0f)};

        m_cam.smoothScaleTowards(gamedata::global::baseCameraScale);
        m_cam.smoothMoveAxisTowards(target, {1.0f, 0.5f}, {1.0f, 1.0f}, {utils::lerp(2.0f, 10.0f, alphas.x), utils::lerp(2.0f, 10.0f, alphas.y)});
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

void CameraSystem::debugDraw(Renderer &ren_, Camera &cam_)
{
    if (ConfigurationManager::instance().m_debug.m_drawCameraOffset)
    {
        auto [trans, phys, dtar] = m_reg.get<ComponentTransform, ComponentPhysical, ComponentDynamicCameraTarget>(m_playerId);
        ren_.drawLine(trans.m_pos, trans.m_pos + dtar.m_offset, {188, 74, 155, 255}, cam_);
        ren_.drawLine(trans.m_pos + dtar.m_offset, trans.m_pos + dtar.m_offset + BODY_OFFSET, {188, 74, 155, 255}, cam_);
    }
}

void CameraSystem::receiveEvents(GAMEPLAY_EVENTS event, const float scale_)
{
    switch (event)
    {
        case (GAMEPLAY_EVENTS::CAM_STOP):
            if (scale_ > 0)
                m_cameraStopped = !m_cameraStopped;
            break;

        default:
        break;
    }
}

#include "CameraFocusArea.h"
#include "Application.h"
#include "GameData.h"

Vector2<float> calcTL(Vector2<float> topLeft_, const Vector2<float> &size_)
{
    if (size_.x < gamedata::global::minCameraSize.x)
        topLeft_.x -= (gamedata::global::minCameraSize.x - size_.x) / 2.0f;

    std::cout << size_.y << " < " << gamedata::global::minCameraSize.y << std::endl;
    if (size_.y < gamedata::global::minCameraSize.y)
        topLeft_.y -= (gamedata::global::minCameraSize.y - size_.y) / 2.0f;

    return topLeft_;
}

CameraFocusArea::CameraFocusArea(const Vector2<int> &topLeft_, const Vector2<int> &size_) :
    CameraFocusArea(calcTL(topLeft_, size_),
        std::max(gamedata::global::minCameraSize.x, size_.x),
        std::max(gamedata::global::minCameraSize.y, size_.y))
{
}

Vector2<float> CameraFocusArea::getCameraTargetPosition(const Vector2<float> &playerFocusPosition_)
{
    return utils::clamp(playerFocusPosition_, m_minCameraPos, m_maxCameraPos);
}

float CameraFocusArea::getScale() const
{
    return m_scale;
}

void CameraFocusArea::draw(const Camera &cam_) const
{
    m_renderer.drawCollider(*this, {0, 255, 0, 0}, {0, 255, 0, 255}, cam_);
    m_renderer.drawCollider(m_useFocusArea, {0, 255, 0, 0}, {0, 255, 0, 50}, cam_);
}

void CameraFocusArea::overrideFocusArea(const Collider &area_)
{
    m_useFocusArea = area_;
}

bool CameraFocusArea::checkIfEnters(const Collider &cld_, bool isOwned_) const
{
    if (isOwned_) // Condition to remain in focus
        return cld_.getOwnOverlapPortion(m_useFocusArea) > 0.3; 
    else // Condition to enter focus
        return cld_.getOwnOverlapPortion(m_useFocusArea) > 0.8; 
}

CameraFocusArea::CameraFocusArea(const Vector2<int> &topLeft_, int scaledSizeX_, int scaledSizeY_) :
    Collider(topLeft_, Vector2{scaledSizeX_, scaledSizeY_}),
    m_renderer(Application::instance().m_renderer),
    m_scale(std::max(gamedata::global::maxCameraSize.x / static_cast<float>(scaledSizeX_), gamedata::global::maxCameraSize.y / static_cast<float>(scaledSizeY_))),
    m_useFocusArea(topLeft_, Vector2{scaledSizeX_, scaledSizeY_})
{
    auto halfCamSize = gamedata::global::maxCameraSize / m_scale / 2.0f;
    m_minCameraPos = m_topLeft + halfCamSize;
    m_maxCameraPos = m_topLeft + m_size - halfCamSize;
}
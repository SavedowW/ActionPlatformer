#include "CameraFocusArea.h"

CameraFocusArea::CameraFocusArea(Vector2<float> center_, Vector2<float> size_, Renderer &renderer_) :
    CameraFocusArea(center_,
        std::max(gamedata::global::minCameraSize.x, size_.x),
        std::max(gamedata::global::minCameraSize.y, size_.y),
        renderer_)
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

void CameraFocusArea::draw(const Camera &cam_)
{
    m_renderer.drawCollider(*this, {0, 255, 0, 0}, 255, cam_);
    m_renderer.drawCollider(m_useFocusArea, {0, 255, 0, 0}, 50, cam_);
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

CameraFocusArea::CameraFocusArea(Vector2<float> center_, float scaledSizeX_, float scaledSizeY_, Renderer &renderer_) :
    Collider(center_, Vector2{scaledSizeX_ / 2.0f, scaledSizeY_ / 2.0f}),
    m_useFocusArea(center_,  Vector2{scaledSizeX_ / 2.0f, scaledSizeY_ / 2.0f}),
    m_renderer(renderer_),
    m_scale(std::min(scaledSizeX_/ gamedata::global::baseResolution.x, scaledSizeY_ / gamedata::global::baseResolution.y))
{
    auto halfCamSize = gamedata::global::baseResolution * (m_scale / 2.0f);
    m_minCameraPos = m_center - m_halfSize + halfCamSize;
    m_maxCameraPos = m_center + m_halfSize - halfCamSize;
}
#include "CameraFocusArea.h"

CameraFocusArea::CameraFocusArea(Vector2<float> pos_, Vector2<float> size_, Renderer &renderer_) :
    CameraFocusArea(pos_.x, pos_.y,
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
}

bool CameraFocusArea::checkIfEnters(const Collider &cld_, bool isOwned_) const
{
    if (isOwned_) // Condition to remain in focus
        return cld_.getOwnOverlapPortion(*this) > 0.3; 
    else // Condition to enter focus
        return cld_.getOwnOverlapPortion(*this) > 0.8; 
}

CameraFocusArea::CameraFocusArea(float x_, float y_, float w_, float h_, Renderer &renderer_) :
    Collider(x_ - w_ / 2.0f, y_ - h_ / 2.0f, w_, h_),
    m_renderer(renderer_),
    m_scale(std::min(w_ / gamedata::global::baseResolution.x, h_ / gamedata::global::baseResolution.y))
{
    auto camSize = gamedata::global::baseResolution * m_scale;
    m_minCameraPos = camSize / 2 + Vector2{x, y};
    m_maxCameraPos = Vector2{x, y} + Vector2{w, h} - camSize / 2;
}
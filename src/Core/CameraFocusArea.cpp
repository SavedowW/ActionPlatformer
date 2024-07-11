#include "CameraFocusArea.h"

CameraFocusArea::CameraFocusArea(Vector2<float> pos_, float h_, Renderer &renderer_) :
    CameraFocusArea(pos_.x, pos_.y,
    gamedata::global::baseResolution.x / gamedata::global::baseResolution.y * std::max(h_, gamedata::global::minCameraSize.y),
    std::max(h_, gamedata::global::minCameraSize.y), renderer_)
    
{
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

CameraFocusArea::CameraFocusArea(float x_, float y_, float w_, float scaledH_, Renderer &renderer_) :
    Collider(x_ - w_ / 2.0f, y_ - scaledH_ / 2.0f, w_, scaledH_),
    m_renderer(renderer_),
    m_scale(scaledH_ / gamedata::global::baseResolution.y)
{
    
}
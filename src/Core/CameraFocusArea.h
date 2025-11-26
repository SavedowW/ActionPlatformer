#ifndef CAMERA_FOCUS_AREA_H_
#define CAMERA_FOCUS_AREA_H_

#include "RectCollider.h"
#include "Renderer.h"
#include "Vector2.hpp"

class CameraFocusArea : public Collider
{
public:
    CameraFocusArea(const Vector2<int> &topLeft_, const Vector2<int> &size_);
    Vector2<float> getCameraTargetPosition(const Vector2<float> &playerFocusPosition_);
    bool checkIfEnters(const Collider &cld_, bool isOwned_) const;
    float getScale() const;
    void draw(const Camera &cam_) const;
    void overrideFocusArea(const Collider &area_);

private:
    CameraFocusArea(const Vector2<int> &topLeft_, int scaledSizeX_, int scaledSizeY_);
    
    Renderer &m_renderer;
    float m_scale;

    Vector2<float> m_minCameraPos;
    Vector2<float> m_maxCameraPos;

    Collider m_useFocusArea;

};

#endif
#ifndef CAMERA_FOCUS_AREA_H_
#define CAMERA_FOCUS_AREA_H_

#include "Renderer.h"
#include "Vector2.h"
#include "GameData.h"

class CameraFocusArea : public Collider
{
public:
    CameraFocusArea() = default;
    CameraFocusArea(CameraFocusArea &&rhs_) = default;
    CameraFocusArea &operator=(CameraFocusArea &&rhs_) = default;
    CameraFocusArea(const CameraFocusArea &rhs_) = delete;
    CameraFocusArea &operator=(const CameraFocusArea &rhs_) = delete;
    
    CameraFocusArea(Vector2<int> topLeft_, Vector2<int> size_, Renderer &renderer_);
    Vector2<float> getCameraTargetPosition(const Vector2<float> &playerFocusPosition_);
    bool checkIfEnters(const Collider &cld_, bool isOwned_) const;
    float getScale() const;
    void draw(const Camera &cam_);
    void overrideFocusArea(const Collider &area_);

private:
    CameraFocusArea(Vector2<int> topLeft_, int scaledSizeX_, int scaledSizeY_, Renderer &renderer_);
    
    Renderer *m_renderer;
    float m_scale;

    Vector2<float> m_minCameraPos;
    Vector2<float> m_maxCameraPos;

    Collider m_useFocusArea;

};

#endif
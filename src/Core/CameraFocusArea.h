#ifndef CAMERA_FOCUS_AREA_H_
#define CAMERA_FOCUS_AREA_H_

#include "Renderer.h"
#include "Vector2.h"
#include "GameData.h"

class CameraFocusArea : public Collider
{
public:
    CameraFocusArea(Vector2<float> pos_, float h_, Renderer &renderer_);
    bool checkIfEnters(const Collider &cld_, bool isOwned_) const;
    float getScale() const;
    void draw(const Camera &cam_);

private:
    CameraFocusArea(float x_, float y_, float w_, float scaledH_, Renderer &renderer_);
    
    Renderer &m_renderer;
    float m_scale;

};

#endif
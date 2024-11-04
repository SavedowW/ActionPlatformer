#ifndef CAMERA_H_
#define CAMERA_H_

#include "Vector2.h"
#include "FrameTimer.h"

class Camera
{
public:
    Camera(const Vector2<float> &pos_, const Vector2<float> &cameraBaseSize_, const Vector2<float> &areaSize_);

    Vector2<float> getPos() const;
    void setPos(const Vector2<float> &pos_);
    Vector2<float> getTopLeft() const;

    Vector2<float> getSize() const;
    void setBaseSize(const Vector2<float> &cameraSize_);
    Vector2<float> getBaseSize() const;

    Vector2<float> getAreaSize() const;
    void setAreaSize(const Vector2<float> &areaSize_);

    float getScale() const;
    void setScale(const float scale_);
    
    void smoothMoveTowards(const Vector2<float> &pos_, const Vector2<float> &deltaMul_, float mindir_, float pow_ = 0.5f, float divider_ = 0.5f);
    void smoothScaleTowards(const float tarScale_, float pow_ = 0.5f, float divider_ = 50.0f);

    Vector2<float> getPositionNormalizedValues();

    void update();
    void startShake(int xAmp, int yAmp, uint32_t period);

private:
    Vector2<float> m_pos;
    Vector2<float> m_cameraBaseSize;
    Vector2<float> m_areaSize;
    float m_scale = 1.0f;

    FrameTimer<false> m_shakeTimer;
    int m_xShakeAmp = 0;
    int m_yShakeAmp = 0;
    Vector2<float> m_thisFrameAmp;

    void normalizePosition();
    Vector2<float> getCamPositionInBoundaries(const Vector2<float> &pos_);
};

#endif
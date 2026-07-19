#pragma once
#include "Vector2.hpp"
#include "FrameTimer.h"

class Camera
{
public:
    Camera(const Vector2<int> &pos_, const Vector2<int> &cameraBaseSize_, const Vector2<int> &areaSize_);

    Vector2<int> getPos() const noexcept;
    void setPos(const Vector2<int> &pos_) noexcept;
    Vector2<int> getTopLeft() const;

    Vector2<int> getSize() const noexcept;
    Vector2<int> getBaseSize() const noexcept;
    Vector2<int> getAreaSize() const noexcept;

    float getScale() const noexcept;
    void setScale(float scale_) noexcept;
    
    void smoothMoveAxisTowards(const Vector2<int> &pos_, const Vector2<float> &deltaMul_, const Vector2<int> &mindir_, const Vector2<float> &dividers_) noexcept;
    void smoothScaleTowards(float tarScale_, float pow_ = 0.5f, float divider_ = 50.0f) noexcept;

    void update() noexcept;
    void startShake(int xAmp, int yAmp, uint32_t period) noexcept;

private:
    Vector2<int> m_pos;
    const Vector2<int> m_cameraBaseSize;
    const Vector2<int> m_areaSize;
    float m_scale = 1.0f;

    FrameTimer<false> m_shakeTimer;
    int m_xShakeAmp = 0;
    int m_yShakeAmp = 0;
    Vector2<int> m_thisFrameAmp;

    void normalizePosition();
    Vector2<int> getCamPositionInBoundaries(const Vector2<int> &pos_) const noexcept;
};

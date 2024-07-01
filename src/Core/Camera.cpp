#include "Camera.h"
#include "GameData.h"

Camera::Camera(const Vector2<float> &pos_, const Vector2<float> &cameraBaseSize_, const Vector2<float> &areaSize_) :
    m_pos(pos_),
    m_cameraBaseSize(cameraBaseSize_),
    m_areaSize(areaSize_),
    m_scale(1.0f)
{
    normalizePosition();
}

Vector2<float> Camera::getPos() const
{
    return m_pos + Vector2{m_thisFrameAmp, 0.0f};
}

void Camera::setPos(const Vector2<float> &pos_)
{
    m_pos = pos_;
    normalizePosition();
}

Vector2<float> Camera::getTopLeft() const
{
    return getPos() - getSize() / 2.0f;
}

Vector2<float> Camera::getSize() const
{
    return m_cameraBaseSize * m_scale;
}

Vector2<float> Camera::getBaseSize() const
{
    return m_cameraBaseSize;
}

void Camera::setBaseSize(const Vector2<float> &cameraSize_)
{
    m_cameraBaseSize = cameraSize_;
    normalizePosition();
}

Vector2<float> Camera::getAreaSize() const
{
    return m_areaSize;
}

void Camera::setAreaSize(const Vector2<float> &areaSize_)
{
    m_areaSize = areaSize_;
    normalizePosition();
}

Vector2<float> Camera::getPositionNormalizedValues()
{
    const auto currentSize = getSize();
    const auto tl = getTopLeft();
    auto maxpos = m_areaSize - currentSize - Vector2{1.0f, 1.0f};
    return {tl.x / maxpos.x, tl.y / maxpos.y};
}

void Camera::normalizePosition()
{
    m_pos = getCamPositionInBoundaries(m_pos);
}

float Camera::getScale() const
{
    return m_scale;
}

void Camera::setScale(const float scale_)
{
    m_scale = utils::clamp(scale_, 0.000001f, gamedata::global::maxCameraScale);
    normalizePosition();
}

void Camera::smoothMoveTowards(const Vector2<float> &pos_, float pow_, float divider_)
{
    auto realTar = getCamPositionInBoundaries(pos_);
    auto deltaVec = realTar - m_pos;
    if (deltaVec.getLen() == 0)
        return;

    auto offset = deltaVec.normalised();
    float offsetLen = pow(deltaVec.getLen(), pow_) / divider_;
    //float offsetLen = deltaVec.getLen() / 2.0f;
    if (offsetLen > deltaVec.getLen())
        offsetLen = deltaVec.getLen();

    //std::cout << "Move by " << offsetLen << std::endl;
    
    offset *= offsetLen;
    //m_pos += offset;
    setPos(m_pos + offset);

}

void Camera::smoothScaleTowards(const float tarScale_, float pow_, float divider_)
{
    auto realTar = utils::clamp(tarScale_, 0.000001f, gamedata::global::maxCameraScale);
    auto delta = realTar - m_scale;

    if (delta == 0)
        return;

    float sign = 1;
    if (delta < 0)
        sign = -1; 
    delta = abs(delta);

    float offsetLen = pow(delta, pow_) / divider_;
    if (offsetLen > delta)
        offsetLen = delta;

    offsetLen *= sign;

    //m_scale += offsetLen;
    setScale(m_scale + offsetLen);

    //std::cout << "Delta: " << delta << " :: Scaled for " << offsetLen << " until " << m_scale;
}

Vector2<float> Camera::getCamPositionInBoundaries(const Vector2<float> &pos_)
{
    const auto currentSize = getSize();
    const auto minPos = currentSize / 2;
    const auto maxPos = m_areaSize - currentSize / 2 - Vector2{1.0f, 1.0f};

    return utils::clamp(pos_, minPos, maxPos);

}

void Camera::update()
{
    if (m_shakeTimer.update())
    {
        m_thisFrameAmp = 0;
        m_xShakeAmp = 0;
    }
    else if (m_xShakeAmp)
    {
        int realAmp = m_xShakeAmp * (1 - m_shakeTimer.getProgressNormalized());
        if (realAmp)
            m_thisFrameAmp = (rand() % realAmp) - (realAmp / 2.0f);
        else
            m_thisFrameAmp = 0;

    }
}

void Camera::startShake(int xAmp, int period)
{
    m_xShakeAmp = xAmp;
    m_shakeTimer.begin(period);
}
#include "Camera.h"
#include "GameData.h"
#include "Profile.h"

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
    return m_pos + m_thisFrameAmp;
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
    return m_cameraBaseSize / m_scale;
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

void Camera::smoothMoveAxisTowards(const Vector2<float> &pos_, const Vector2<float> &deltaMul_, const Vector2<float> &mindir_, const Vector2<float> &dividers_)
{
    auto realTar = getCamPositionInBoundaries(pos_);
    auto deltaVec = (realTar - m_pos).mulComponents(deltaMul_);
    if (abs(deltaVec.x) < mindir_.x)
        deltaVec.x = 0;
    if (abs(deltaVec.y) < mindir_.y)
        deltaVec.y = 0;

    if (deltaVec.x == 0 && deltaVec.y == 0)
        return;

    auto realAbsOffset = Vector2{abs(deltaVec.x), abs(deltaVec.y)};
    realAbsOffset.x = realAbsOffset.x / dividers_.x;
    realAbsOffset.y = realAbsOffset.y / dividers_.y;

    Vector2<float> offset;
    offset.x = utils::signof(deltaVec.x) * utils::clampMaxPriority<float>(realAbsOffset.x, 0.25f, abs(deltaVec.x));
    offset.y = utils::signof(deltaVec.y) * utils::clampMaxPriority<float>(realAbsOffset.y, 0.25f, abs(deltaVec.y));

    //m_pos += offset;
    setPos(m_pos + offset);

    //std::cout << "Camera pos: " << m_pos << std::endl;
}

void Camera::smoothMoveTowards(const Vector2<float> &pos_, const Vector2<float> &deltaMul_, float mindir_, float pow_, float divider_)
{
    auto realTar = getCamPositionInBoundaries(pos_);
    auto deltaVec = (realTar - m_pos).mulComponents(deltaMul_);
    auto dlen = deltaVec.getLen();
    if (dlen <= mindir_)
        return;

    auto offset = deltaVec.normalised();
    float offsetLen = pow(dlen, pow_) / divider_;
    if (offsetLen > dlen)
        offsetLen = dlen;

    //std::cout << "Move by " << offsetLen << std::endl;
    
    offset *= offsetLen;

    offset.x = utils::signof(offset.x) * utils::clampMaxPriority<float>(abs(offset.x), 0.25f, abs(deltaVec.x));
    offset.y = utils::signof(offset.y) * utils::clampMaxPriority<float>(abs(offset.y), 0.25f, abs(deltaVec.y));

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
    PROFILE_FUNCTION;

    if (m_shakeTimer.update())
    {
        m_thisFrameAmp = {0.0f, 0.0f};
        m_xShakeAmp = 0;
        m_yShakeAmp = 0;
    }
    else
    {
        if (m_xShakeAmp)
        {
            int realAmp = m_xShakeAmp * (1 - m_shakeTimer.getProgressNormalized());
            if (realAmp)
                m_thisFrameAmp.x = (rand() % realAmp) - (realAmp / 2.0f);
            else
                m_thisFrameAmp.x = 0;

        }

        if (m_yShakeAmp)
        {
            int realAmp = m_yShakeAmp * (1 - m_shakeTimer.getProgressNormalized());
            if (realAmp)
                m_thisFrameAmp.y = (rand() % realAmp) - (realAmp / 2.0f);
            else
                m_thisFrameAmp.y = 0;

        }
    }
}

void Camera::startShake(int xAmp, int yAmp, uint32_t period)
{
    m_xShakeAmp = xAmp;
    m_yShakeAmp = yAmp;
    m_shakeTimer.begin(period);
}

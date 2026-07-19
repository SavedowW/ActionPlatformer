#include "Camera.h"

#include <algorithm>
#include "GameData.h"
#include "Profile.h"
#include "Logger.hpp"

Camera::Camera(const Vector2<int> &pos_, const Vector2<int> &cameraBaseSize_, const Vector2<int> &areaSize_) :
    m_pos{pos_},
    m_cameraBaseSize{cameraBaseSize_},
    m_areaSize{areaSize_}
{
    normalizePosition();
}

Vector2<int> Camera::getPos() const noexcept
{
    return m_pos + m_thisFrameAmp;
}

void Camera::setPos(const Vector2<int> &pos_) noexcept
{
    m_pos = getCamPositionInBoundaries(pos_);
}

Vector2<int> Camera::getTopLeft() const
{
    return getPos() - getSize() / 2;
}

Vector2<int> Camera::getSize() const noexcept
{
    return m_cameraBaseSize / m_scale;
}

Vector2<int> Camera::getBaseSize() const noexcept
{
    return m_cameraBaseSize;
}

Vector2<int> Camera::getAreaSize() const noexcept
{
    return m_areaSize;
}

void Camera::normalizePosition()
{
    m_pos = getCamPositionInBoundaries(m_pos);
}

float Camera::getScale() const noexcept
{
    return m_scale;
}

void Camera::setScale(const float scale_) noexcept
{
    m_scale = utils::clamp(scale_, 0.000001f, gamedata::global::maxCameraScale);
    normalizePosition();
}

void Camera::smoothMoveAxisTowards(const Vector2<int> &pos_, const Vector2<float> &deltaMul_, const Vector2<int> &mindir_, const Vector2<float> &dividers_) noexcept
{
    const auto realTar = getCamPositionInBoundaries(pos_);
    Vector2<float> deltaVec = (realTar - m_pos).mulComponents(deltaMul_);
    if (abs(deltaVec.x) < mindir_.x)
        deltaVec.x = 0.f;
    if (abs(deltaVec.y) < mindir_.y)
        deltaVec.y = 0.f;

    if (deltaVec.x == 0 && deltaVec.y == 0)
        return;

    const Vector2 realAbsOffset{
        abs(deltaVec.x) / dividers_.x, 
        abs(deltaVec.y) / dividers_.y
    };

    const Vector2<float> offset{
        utils::signof(deltaVec.x) * utils::clampMaxPriority<float>(realAbsOffset.x, 0.25f, abs(deltaVec.x)),
        utils::signof(deltaVec.y) * utils::clampMaxPriority<float>(realAbsOffset.y, 0.25f, abs(deltaVec.y))
    };

    setPos(m_pos + offset);

    //std::cout << "Camera pos: " << m_pos << std::endl;
}

void Camera::smoothScaleTowards(const float tarScale_, float pow_, float divider_) noexcept
{
    auto realTar = utils::clamp(tarScale_, 0.000001f, gamedata::global::maxCameraScale);
    auto delta = realTar - m_scale;

    if (delta == 0)
        return;

    float sign = 1;
    if (delta < 0)
        sign = -1; 
    delta = abs(delta);

    const float offsetLen = sign * std::min(
        pow(delta, pow_) / divider_,
        delta
    );

    //m_scale += offsetLen;
    setScale(m_scale + offsetLen);

    //std::cout << "Delta: " << delta << " :: Scaled for " << offsetLen << " until " << m_scale;
}

Vector2<int> Camera::getCamPositionInBoundaries(const Vector2<int> &pos_) const noexcept
{
    const auto currentHalfSize = getSize() / 2;
    const auto minPos = currentHalfSize;
    const auto maxPos = m_areaSize - currentHalfSize - Vector2{1, 1};

    return utils::clamp(pos_, minPos, maxPos);

}

void Camera::update() noexcept
{
    if (m_shakeTimer.update())
    {
        m_thisFrameAmp = {0, 0};
        m_xShakeAmp = 0;
        m_yShakeAmp = 0;
    }
    else
    {
        if (m_xShakeAmp)
        {
            const int realAmp = static_cast<int>(static_cast<float>(m_xShakeAmp) * (1.0f - m_shakeTimer.getProgressNormalized()));
            if (realAmp)
                m_thisFrameAmp.x = (rand() % realAmp) - (realAmp / 2);
            else
                m_thisFrameAmp.x = 0;

        }

        if (m_yShakeAmp)
        {
            const int realAmp = static_cast<int>(static_cast<float>(m_yShakeAmp) * (1 - m_shakeTimer.getProgressNormalized()));
            if (realAmp)
                m_thisFrameAmp.y = (rand() % realAmp) - (realAmp / 2);
            else
                m_thisFrameAmp.y = 0;

        }
    }
}

void Camera::startShake(int xAmp, int yAmp, uint32_t period) noexcept
{
    m_xShakeAmp = xAmp;
    m_yShakeAmp = yAmp;
    m_shakeTimer.begin(period);
}

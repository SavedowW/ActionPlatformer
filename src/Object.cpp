#include "Object.h"

Object::Object(Application &application_, Vector2<float> pos_) :
    m_pos(pos_),
    m_renderer(*application_.getRenderer())
{
}

void Object::setOnLevel(Application &application_)
{
    loadAnimations(application_);
}

void Object::update()
{
    m_velocity += getCurrentGravity();

    auto drag = getInertiaDrag();

    if (m_inertia.x != 0)
    {
        auto absInertia = abs(m_inertia.x);
        auto m_inertiaSign = m_inertia.x / abs(m_inertia.x);
        absInertia = std::max(absInertia - drag.x, 0.0f);
        m_inertia.x = m_inertiaSign * absInertia;
    }

    if (m_inertia.y != 0)
    {
        auto absInertia = abs(m_inertia.y);
        auto m_inertiaSign = m_inertia.y / abs(m_inertia.y);
        absInertia = std::max(absInertia - drag.y, 0.0f);
        m_inertia.y = m_inertiaSign * absInertia;
    }
}

Vector2<float> &Object::accessInertia()
{
    return m_inertia;
}

Vector2<float> &Object::accessVelocity()
{
    return m_velocity;
}

Vector2<float> &Object::accessPos()
{
    return m_pos;
}

Vector2<float> Object::getOwnHorDir() const
{
    switch (m_ownOrientation)
    {
        case ORIENTATION::LEFT:
            return {-1.0f, 0.0f};

        case ORIENTATION::RIGHT:
            return {1.0f, 0.0f};

        default:
            return {0.0f, 0.0f};
    }
}

void Object::velocityToInertia()
{
    m_inertia += m_velocity;
    m_velocity = {0.0f, 0.0f};
}

Vector2<float> Object::getInertiaDrag() const
{
    return {1.0f, 0.0f};
}

Vector2<float> Object::getInertiaMultiplier() const
{
    return {1.0f, 1.0f};
}

Vector2<float> Object::getPosOffest() const
{
    return m_velocity + m_inertia.mulComponents(getInertiaMultiplier());
}

ORIENTATION Object::getOwnOrientation() const
{
    return m_ownOrientation;
}

void Object::setOwnOrientation(ORIENTATION or_)
{
    m_ownOrientation = or_;
}

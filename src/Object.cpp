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

    if (m_inertia.x != 0)
    {
        auto absInertia = abs(m_inertia.x);
        auto m_inertiaSign = m_inertia.x / abs(m_inertia.x);
        absInertia = std::max(absInertia - getInertiaDrag(), 0.0f);
        m_inertia.x = m_inertiaSign * absInertia;
    }
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

float Object::getInertiaDrag() const
{
    return 1.0f;
}

Vector2<float> Object::getPosOffest() const
{
    return m_velocity + m_inertia;
}

ORIENTATION Object::getOwnOrientation() const
{
    return m_ownOrientation;
}

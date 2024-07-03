#include "Object.h"

Object::Object(Application &application_, Vector2<float> pos_) :
    m_pos(pos_),
    m_renderer(*application_.getRenderer())
{
    m_gravity = {0, 0.005f};
}

void Object::setOnLevel(Application &application_)
{
    loadAnimations(application_);
}

void Object::update()
{
    m_velocity += m_gravity;
}

Vector2<float> &Object::accessVelocity()
{
    return m_velocity;
}

Vector2<float> &Object::accessPos()
{
    return m_pos;
}

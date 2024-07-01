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
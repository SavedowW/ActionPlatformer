#ifndef OBJECT_H_
#define OBJECT_H_

#include "Application.h"
#include "StateMarker.h"

class Object
{
public:
    Object(Application &application_, Vector2<float> pos_);

    virtual void setOnLevel(Application &application_);

    virtual void update();
    virtual void draw(Camera &cam_) = 0;
    Vector2<float> &accessVelocity();
    Vector2<float> &accessPos();

    ORIENTATION getOwnOrientation() const;

    virtual Collider getPushbox() const = 0;

    virtual ~Object() = default;

protected:
    virtual void loadAnimations(Application &application_) = 0;

    Renderer &m_renderer;
    std::map<int, std::unique_ptr<Animation>> m_animations;

    Vector2<float> m_pos;
    Vector2<float> m_velocity;
    Vector2<float> m_gravity;

    ORIENTATION m_ownOrientation = ORIENTATION::RIGHT;

};

#endif
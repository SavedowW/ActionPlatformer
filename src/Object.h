#ifndef OBJECT_H_
#define OBJECT_H_

#include "Application.h"
#include "StateMarker.h"

class Object
{
public:
    Object(Application &application_);

    virtual void setOnLevel(Application &application_, Vector2<float> pos_);

    virtual void update();
    virtual void draw(Camera &cam_) = 0;
    Vector2<float> &accessInertia();
    Vector2<float> &accessVelocity();
    Vector2<float> &accessPos();
    Vector2<float> getOwnHorDir() const;
    void velocityToInertia();
    virtual Vector2<float> getInertiaDrag() const;
    virtual Vector2<float> getInertiaMultiplier() const;
    Vector2<float> getPosOffest() const;

    ORIENTATION getOwnOrientation() const;
    void setOwnOrientation(ORIENTATION or_);

    virtual Collider getPushbox() const = 0;

    virtual ~Object() = default;

protected:
    virtual void loadAnimations(Application &application_) = 0;
    virtual Vector2<float> getCurrentGravity() const = 0;

    Renderer &m_renderer;
    std::map<int, std::unique_ptr<Animation>> m_animations;

    Vector2<float> m_pos;
    Vector2<float> m_velocity;
    Vector2<float> m_inertia;

    ORIENTATION m_ownOrientation = ORIENTATION::RIGHT;

};

#endif
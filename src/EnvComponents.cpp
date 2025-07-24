#include "EnvComponents.h"

const Collider GrassTopComp::m_colliderRight{Vector2{14, -6}, Vector2{2, 6}};
const Collider GrassTopComp::m_colliderLeft{Vector2{-14, -6}, Vector2{2, 6}};
int GrassTopComp::m_idleAnimId = 0;
int GrassTopComp::m_flickLeftAnimId = 0;
int GrassTopComp::m_flickRightAnimId = 0;

void GrassTopComp::touchedPlayer(const Vector2<float> &velocity_, EntityAnywhere self_)
{
    switch (m_state)
    {
        case State::IDLE:
            if (velocity_.x >= 2.2f)
            {
                auto &renderable = self_.reg->get<ComponentAnimationRenderable>(self_.idx);
                renderable.m_currentAnimation = &renderable.m_animations.at(m_flickRightAnimId);
                renderable.m_currentAnimation->reset();
                m_state = State::FLICK_RIGHT;
            }
            else if (velocity_.x <= -2.2f)
            {
                auto &renderable = self_.reg->get<ComponentAnimationRenderable>(self_.idx);
                renderable.m_currentAnimation = &renderable.m_animations.at(m_flickLeftAnimId);
                renderable.m_currentAnimation->reset();
                m_state = State::FLICK_LEFT;
            }
            break;
    }
}

bool GrassTopComp::update(EntityAnywhere self_)
{
    if (m_state != State::IDLE)
    {
        auto &renderable = self_.reg->get<ComponentAnimationRenderable>(self_.idx);
        if (renderable.m_currentAnimation->isFinished())
        {
            renderable.m_currentAnimation = &renderable.m_animations.at(m_idleAnimId);
            renderable.m_currentAnimation->reset();
            m_state = State::IDLE;
        }
    }

    return m_state == State::IDLE;
}

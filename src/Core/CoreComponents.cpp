#include "CoreComponents.h"

Vector2<float> ComponentTransform::getOwnHorDir() const
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

ComponentPhysical::ComponentPhysical(const CollisionArea &cldArea_) :
    m_collisionArea(cldArea_)
{
}

void ComponentPhysical::onUpdate()
{
    auto &transform = getComponent<ComponentTransform>();

    m_velocity += m_gravity;

    if (m_inertia.x != 0)
    {
        auto absInertia = abs(m_inertia.x);
        auto m_inertiaSign = m_inertia.x / abs(m_inertia.x);
        absInertia = std::max(absInertia - m_drag.x, 0.0f);
        m_inertia.x = m_inertiaSign * absInertia;
    }

    if (m_inertia.y != 0)
    {
        auto absInertia = abs(m_inertia.y);
        auto m_inertiaSign = m_inertia.y / abs(m_inertia.y);
        absInertia = std::max(absInertia - m_drag.y, 0.0f);
        m_inertia.y = m_inertiaSign * absInertia;
    }
}

void ComponentPhysical::velocityToInertia()
{
    m_inertia += m_velocity;
    m_velocity = {0.0f, 0.0f};
}

Collider ComponentPhysical::getPushbox()
{
    return m_pushbox + getComponent<ComponentTransform>().m_pos;
}

Vector2<float> ComponentPhysical::getPosOffest() const
{
    return m_velocity + m_inertia.mulComponents(m_inertiaMultiplier);
}

void ComponentObstacleFallthrough::onUpdate()
{
    m_isIgnoringObstacles.update();
}

void ComponentObstacleFallthrough::setIgnoringObstacles()
{
    m_isIgnoringObstacles.begin(5);
}

bool ComponentObstacleFallthrough::isIgnoringAllObstacles() const
{
    return m_isIgnoringObstacles.isActive();
}

// true if not ignored
bool ComponentObstacleFallthrough::touchedObstacleTop(int obstacleId_)
{
    if (m_isIgnoringObstacles.isActive())
    {
        m_ignoredObstacles.insert(obstacleId_);
        return false;
    }

    return !m_ignoredObstacles.contains(obstacleId_);
}

bool ComponentObstacleFallthrough::touchedObstacleSlope(int obstacleId_)
{
    if (m_isIgnoringObstacles.isActive())
    {
        m_ignoredObstacles.insert(obstacleId_);
        return false;
    }
    return !m_ignoredObstacles.contains(obstacleId_);
}

bool ComponentObstacleFallthrough::checkIgnoringObstacle(int obstacleId_) const
{
    return m_isIgnoringObstacles.isActive() || m_ignoredObstacles.contains(obstacleId_);
}

bool ComponentObstacleFallthrough::touchedObstacleBottom(int obstacleId_)
{
    m_ignoredObstacles.insert(obstacleId_);
    return false;
}

bool ComponentObstacleFallthrough::touchedObstacleSide(int obstacleId_)
{
    m_ignoredObstacles.insert(obstacleId_);
    return false;
}
void ComponentObstacleFallthrough::cleanIgnoredObstacles()
{
    auto touched = getComponent<ComponentPhysical>().m_collisionArea.getPlayerTouchingObstacles(getComponent<ComponentPhysical>().getPushbox());
    std::set<int> res;
    std::set_intersection(
        m_ignoredObstacles.begin(), m_ignoredObstacles.end(),
        touched.begin(), touched.end(),
        std::inserter(res, res.begin()));

    m_ignoredObstacles = res;
}

bool ComponentPhysical::attemptResetGround()
{
    auto &transform = getComponent<ComponentTransform>();
    auto pb = getPushbox();
    if (m_magnetLimit <= 0.0f)
        return false;

    float height = transform.m_pos.y;
    if ( m_collisionArea.getHighestVerticalMagnetCoord(pb, height, getComponent<ComponentObstacleFallthrough>().m_ignoredObstacles))
    {
        float magnetRange = height - transform.m_pos.y;
        if (magnetRange <= m_magnetLimit)
        {
            std::cout << "MAGNET: " << magnetRange << std::endl;
            transform.m_pos.y = height;
            return true;
        }
    }

    return false;
}

ComponentAnimationRenderable::ComponentAnimationRenderable(Renderer *renderer_) :
    m_renderer(renderer_)
{
}

void ComponentAnimationRenderable::draw(Camera &cam_)
{
    auto &transform = getComponent<ComponentTransform>();
    if (m_currentAnimation != nullptr)
    {
        auto texSize = m_currentAnimation->getSize();
        auto animorigin = m_currentAnimation->getOrigin();
        auto texPos = transform.m_pos;
        texPos.y -= animorigin.y;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (transform.m_ownOrientation == ORIENTATION::LEFT)
        {
            flip = SDL_FLIP_HORIZONTAL;
            texPos.x -= (texSize.x - animorigin.x);
        }
        else
        {
            texPos.x -= animorigin.x;
        }

        auto spr = m_currentAnimation->getSprite();
        auto edge = m_currentAnimation->getBorderSprite();

        m_renderer->renderTexture(spr, texPos.x, texPos.y, texSize.x , texSize.y, cam_, 0.0f, flip);

        if (gamedata::debug::drawColliders)
        {
            m_renderer->drawCollider(getComponent<ComponentPhysical>().getPushbox(), {238, 195, 154, 50}, 100, cam_);
        }
    }
}

void ComponentAnimationRenderable::onUpdate()
{
    m_currentAnimation->update();
}

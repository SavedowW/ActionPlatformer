#include "PhysicsSystem.h"
#include "Core/CoreComponents.h"
#include "Core/Profile.h"
#include <stack>

const float PhysicsEntityHandler::VerticalOffsetLimitMul = 1.3f;

PhysicsEntityHandler::PhysicsEntityHandler(const CollidersView &cld_, ComponentTransform &trans_, 
        ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, WorldPosition &worldPos_) :
    m_cld{cld_},
    m_trans{trans_},
    m_phys{phys_},
    m_pushbox{phys_.pushbox},
    m_obsFallthrough{obsFallthrough_},
    m_worldPos{worldPos_}
{  
}

enum class AttemptType : uint8_t
{
    // Next attempt can be of any type, but cannot be further than loopback allows. Initial is considered BACKWARD too. Movement is halted on each non-initial BACKWARD attempt
    BACKWARD,

    // RequireMagnet is false for this specific case. Only upward and backward attempts are allowed
    UPWARD,

    // Only downward and backward attempts are allowed
    DOWNWARD
};

struct AttemptPos
{
    AttemptPos(const Vector2<int> &pos_, bool requireMagnet_, bool haltMovement_, AttemptType attemptType_) :
        pos{pos_},
        requireMagnet{requireMagnet_},
        haltMovement{haltMovement_},
        attemptType{attemptType_}
    {}

    const Vector2<int> pos;
    bool requireMagnet;
    bool haltMovement;
    AttemptType attemptType;
};

void PhysicsEntityHandler::moveRight(const int offset_)
{
    assert(offset_ > 0);

    const auto startingPos = m_trans.m_pos;

    const auto halfWidth = m_pushbox.m_size.x / 2;

    const auto xLoopbackLimit = startingPos.x + 1;

    std::stack<AttemptPos> attempts;
    attempts.emplace(startingPos.add(offset_, 0), true, false, AttemptType::BACKWARD);

    while (!attempts.empty())
    {
        const auto attempt = attempts.top();
        attempts.pop();
        bool valid = true;

        for (const auto& [idx, cld] : m_cld.each())
        {
            if (!cld.m_isEnabled)
                continue;

            const auto newPb = m_pushbox + attempt.pos;

            int highest = 0;
            const auto overlap = cld.m_resolved.checkOverlap(newPb, highest);
            if ((overlap & OverlapResult::OVERLAP_BOTH) != OverlapResult::OVERLAP_BOTH)
                continue;

            const Vector2<int> topPos{attempt.pos.x, highest - 1};

            // Here and below +1 is added because slopes rely on floating point math and that work bad when getting onto the slope
            const bool upCondition = static_cast<float>(startingPos.y - topPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(topPos.x - startingPos.x);

            // If we can't get on top of the obstacle, then we don't bother moving around it
            if (cld.m_obstacleId && (m_obsFallthrough.checkIgnoringObstacle(cld.m_obstacleId) || !upCondition))
                continue;

            valid = false;
            const auto leftmost = cld.m_resolved.getMostLeftAt(newPb) - halfWidth - 1;
            const Vector2<int> bottomPos{attempt.pos.x, cld.m_resolved.bottomY() + m_pushbox.m_size.y};
            
            switch (attempt.attemptType)
            {
                case AttemptType::BACKWARD:
                    if (leftmost >= xLoopbackLimit)
                        attempts.emplace(Vector2{leftmost, attempt.pos.y}, true, true, AttemptType::BACKWARD);
                    if (static_cast<float>(bottomPos.y - startingPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(bottomPos.x - startingPos.x))
                        attempts.emplace(bottomPos, true, attempt.haltMovement, AttemptType::DOWNWARD);
                    if (upCondition)
                        attempts.emplace(topPos, false, attempt.haltMovement, AttemptType::UPWARD);
                    break;

                case AttemptType::UPWARD:
                    if (leftmost >= xLoopbackLimit)
                        attempts.emplace(Vector2{leftmost, attempt.pos.y}, true, true, AttemptType::UPWARD);
                    if (upCondition)
                        attempts.emplace(topPos, false, attempt.haltMovement, AttemptType::UPWARD);
                    break;

                case AttemptType::DOWNWARD:
                    if (leftmost >= xLoopbackLimit)
                        attempts.emplace(Vector2{leftmost, attempt.pos.y}, true, true, AttemptType::DOWNWARD);
                    if (static_cast<float>(bottomPos.y - startingPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(bottomPos.x - startingPos.x))
                        attempts.emplace(bottomPos, true, attempt.haltMovement, AttemptType::DOWNWARD);
                    break;
            }

            break;
        }

        if (valid)
        {
            m_trans.m_pos = attempt.pos;
            m_requireMagnet = attempt.requireMagnet;
            if (attempt.haltMovement)
            {
                m_phys.velocity.x = 0;
                m_phys.inertia.x = 0;
            }
            return;
        }
    }
}

void PhysicsEntityHandler::moveLeft(const int offset_)
{
    assert(offset_ > 0);

    const auto startingPos = m_trans.m_pos;

    const auto halfWidth = m_pushbox.m_size.x / 2;

    const auto xLoopbackLimit = startingPos.x - 1;

    std::stack<AttemptPos> attempts;
    attempts.emplace(startingPos.sub(offset_, 0), true, false, AttemptType::BACKWARD);

    while (!attempts.empty())
    {
        const auto attempt = attempts.top();
        attempts.pop();
        bool valid = true;

        for (const auto& [idx, cld] : m_cld.each())
        {
            if (!cld.m_isEnabled)
                continue;

            const auto newPb = m_pushbox + attempt.pos;

            int highest = 0;
            const auto overlap = cld.m_resolved.checkOverlap(newPb, highest);
            if ((overlap & OverlapResult::OVERLAP_BOTH) != OverlapResult::OVERLAP_BOTH)
                continue;

            const Vector2<int> topPos{attempt.pos.x, highest - 1};

            // Here and below +1 is added because slopes rely on floating point math and that work bad when getting onto the slope
            const bool upCondition = static_cast<float>(startingPos.y - topPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(startingPos.x - topPos.x);

            // If we can't get on top of the obstacle, then we don't bother moving around it
            if (cld.m_obstacleId && (m_obsFallthrough.checkIgnoringObstacle(cld.m_obstacleId) || !upCondition))
                continue;

            valid = false;
            const auto rightmost = cld.m_resolved.getMostRightAt(newPb) + halfWidth;
            const Vector2<int> bottomPos{attempt.pos.x, cld.m_resolved.bottomY() + m_pushbox.m_size.y};
            
            switch (attempt.attemptType)
            {
                case AttemptType::BACKWARD:
                    if (rightmost <= xLoopbackLimit)
                        attempts.emplace(Vector2{rightmost, attempt.pos.y}, true, true, AttemptType::BACKWARD);
                    if (static_cast<float>(bottomPos.y - startingPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(startingPos.x - bottomPos.x))
                        attempts.emplace(bottomPos, true, attempt.haltMovement, AttemptType::DOWNWARD);
                    if (upCondition)
                        attempts.emplace(topPos, false, attempt.haltMovement, AttemptType::UPWARD);
                    break;

                case AttemptType::UPWARD:
                    if (rightmost <= xLoopbackLimit)
                        attempts.emplace(Vector2{rightmost, attempt.pos.y}, true, true, AttemptType::UPWARD);
                    if (upCondition)
                        attempts.emplace(topPos, false, attempt.haltMovement, AttemptType::UPWARD);
                    break;

                case AttemptType::DOWNWARD:
                    if (rightmost <= xLoopbackLimit)
                        attempts.emplace(Vector2{rightmost, attempt.pos.y}, true, true, AttemptType::DOWNWARD);
                    if (static_cast<float>(bottomPos.y - startingPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(startingPos.x - bottomPos.x))
                        attempts.emplace(bottomPos, true, attempt.haltMovement, AttemptType::DOWNWARD);
                    break;
            }

            break;
        }

        if (valid)
        {
            m_trans.m_pos = attempt.pos;
            m_requireMagnet = attempt.requireMagnet;
            if (attempt.haltMovement)
            {
                m_phys.velocity.x = 0;
                m_phys.inertia.x = 0;
            }
            return;
        }
    }
}

void PhysicsEntityHandler::moveDown(int offset_)
{
    assert(offset_ > 0);

    m_requireMagnet = false;
    
    auto newPos = m_trans.m_pos.add(0, offset_);

    bool positionConfirmed = false;
    while (!positionConfirmed)
    {
        positionConfirmed = true;
        
        for (const auto& [idx, cld] : m_cld.each())
        {
            if (!cld.m_isEnabled)
                continue;

            const auto newPb = m_pushbox + newPos;

            int highest = 0;
            const auto overlap = cld.m_resolved.checkOverlap(newPb, highest);
            if ((overlap & OverlapResult::OVERLAP_BOTH) != OverlapResult::OVERLAP_BOTH)
                continue;

            if (cld.m_obstacleId && m_obsFallthrough.checkIgnoringObstacle(cld.m_obstacleId))
                continue;

            m_phys.velocity.y = 0;
            m_phys.inertia.y = 0;

            positionConfirmed = false;
            newPos.y = highest - 1;
            break;
        }
    }

    m_trans.m_pos = newPos;
}

void PhysicsEntityHandler::moveUp(int offset_)
{
    assert(offset_ > 0);

    m_requireMagnet = false;
    
    auto newPos = m_trans.m_pos.sub(0, offset_);

    bool positionConfirmed = false;
    while (!positionConfirmed)
    {
        positionConfirmed = true;
        
        for (const auto& [idx, cld] : m_cld.each())
        {
            if (!cld.m_isEnabled)
                continue;

            const auto newPb = m_pushbox + newPos;

            const auto overlap = cld.m_resolved.checkOverlap(newPb);
            if ((overlap & OverlapResult::OVERLAP_BOTH) != OverlapResult::OVERLAP_BOTH)
                continue;

            if (cld.m_obstacleId)
                continue;

            positionConfirmed = false;
            newPos.y = cld.m_resolved.bottomY() + m_pushbox.m_size.y;
            break;
        }
    }

    m_trans.m_pos = newPos;
}

void PhysicsEntityHandler::magnet()
{
    if (!m_requireMagnet || !m_worldPos.ground.demand)
        return;

    m_requireMagnet = false;

    int height = m_trans.m_pos.y;
    const auto *pcld = getHighestVerticalMagnetCoord(height);
    if ( pcld )
    {
        const auto magnetRange = static_cast<unsigned int>(height - m_trans.m_pos.y - 1);
        if (magnetRange <= m_phys.magnetLimit)
        {
            //std::cout << "MAGNET: " << magnetRange << std::endl;
            m_trans.m_pos.y = height - 1;
            m_phys.velocity.y = 0;
            m_phys.inertia.y = 0;
        }
    }
}

void PhysicsEntityHandler::discoverPosition()
{
    m_worldPos.reset();

    m_obsFallthrough.m_ignoredObstacles.clear();

    const auto pushbox = m_pushbox + m_trans.m_pos;

    for (const auto& [idx, cld] : m_cld.each())
    {
        if (!cld.m_isEnabled)
            continue;

        int highest = m_trans.m_pos.y;
        const auto overlap = cld.m_resolved.checkOverlap(pushbox, highest);

        if ((overlap & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH)
        {
            if (cld.m_obstacleId)
                m_obsFallthrough.setIgnoreObstacle(cld.m_obstacleId);
        }
        else
        {
            if (m_worldPos.ground.demand)
            {
                if ((overlap & OverlapResult::OVERLAP_X) == OverlapResult::OVERLAP_X
                    && (!cld.m_obstacleId || !m_obsFallthrough.checkIgnoringObstacle(cld.m_obstacleId)))
                {
                    if (highest - 1 == m_trans.m_pos.y && (m_worldPos.ground.onGround == entt::null || m_worldPos.ground.onSlopeWithAngle != 0.0f))
                    {
                        m_worldPos.ground.onGround = idx;
                        m_worldPos.ground.onSlopeWithAngle = cld.m_resolved.topAngleCoef();
                    }
                }
            }

            if (m_worldPos.wall.demand)
            {
                const Vector2<int> rightPoint{pushbox.getRightEdge() + 1, pushbox.getTopEdge() + pushbox.m_size.y / 2};
                const Vector2<int> leftPoint{pushbox.getLeftEdge() - 1, pushbox.getTopEdge() + pushbox.m_size.y / 2};

                if (cld.m_resolved.leftX() == rightPoint.x && cld.m_resolved.leftY() <= rightPoint.y && cld.m_resolved.bottomY() >= rightPoint.y)
                    m_worldPos.wall.rightWall = idx;

                if (cld.m_resolved.rightX() == leftPoint.x && cld.m_resolved.rightY() <= leftPoint.y && cld.m_resolved.bottomY() >= leftPoint.y)
                    m_worldPos.wall.leftWall = idx;
            }
        }
    }

    if (m_worldPos.ground.onGround != entt::null)
    {
        m_phys.velocity.y = 0;
        m_phys.inertia.y = 0;
    }

    m_obsFallthrough.m_isIgnoringObstacles.update();
}

const SlopeCollider *PhysicsEntityHandler::getHighestVerticalMagnetCoord(int &coord_)
{
    const auto baseCoord = coord_;
    entt::entity foundGround = entt::null;
    const SlopeCollider *foundcld = nullptr;

    auto pushbox = m_pushbox + m_trans.m_pos;
    
    for (const auto [idx, areaCld_] : m_cld.each())
    {
        if (!areaCld_.m_isEnabled || areaCld_.m_obstacleId && m_obsFallthrough.checkIgnoringObstacle(areaCld_.m_obstacleId))
            continue;

        int height = 0;
        auto horOverlap = areaCld_.m_resolved.checkOverlap(pushbox, height);
        if ((horOverlap & OverlapResult::OVERLAP_X) == OverlapResult::OVERLAP_X)
        {
            if (height >= baseCoord && (foundGround == entt::null || height < coord_))
            {
                coord_ = height;
                foundGround = idx;
                foundcld = &areaCld_.m_resolved;
            }
        }
    }

    return foundcld;
}


PhysicsSystem::PhysicsSystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void PhysicsSystem::prepHitstop()
{
    auto viewPhys = m_reg.view<ComponentPhysical>();
    for (auto [idx, phys] : viewPhys.each())
    {
        if (phys.hitstopLeft)
            phys.hitstopLeft--;
    }
}

void PhysicsSystem::prepEntities()
{
    auto viewPhys = m_reg.view<ComponentPhysical>();
    for (const auto &[idx, phys] : viewPhys.each())
    {
        phys.onMovingPlatform = false;
    }
}

void PhysicsSystem::updatePhysics()
{
    PROFILE_FUNCTION;

    auto viewPhys = m_reg.view<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, WorldPosition>();
    const auto viewscld = m_reg.view<ComponentStaticCollider>();

    for (auto [idx, trans, phys, obsfall, ev] : viewPhys.each())
    {
        if (phys.hitstopLeft)
            continue;

        proceedEntity(viewscld, trans, phys, obsfall, ev);
    }

    /* TODO: notably faster in release build, but harder to debug even with seq, might add debug flags to enable parallel execution
    auto iteratable = viewPhys.each();
    std::for_each(std::execution::par, iteratable.begin(), iteratable.end(), [this, &viewscld](auto inp)
    {
        auto [idx, trans, phys, obsfall, ev] = inp;
        proceedEntity(viewscld, trans, phys, obsfall, ev);
    });
    */
}

void PhysicsSystem::proceedEntity(const CollidersView &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, WorldPosition &worldPos_)
{
    const auto oldPos = trans_.m_pos;

    PhysicsEntityHandler handler{clds_, trans_, phys_, obsFallthrough_, worldPos_};

    // Common stuff
    phys_.velocity += phys_.gravity;

    if (phys_.inertia.x != 0)
    {
        auto absInertia = abs(phys_.inertia.x);
        const auto m_inertiaSign = utils::signof(phys_.inertia.x / abs(phys_.inertia.x));
        absInertia = std::max(absInertia - phys_.drag.x, 0.0f);
        phys_.inertia.x = m_inertiaSign * absInertia;
    }

    if (phys_.inertia.y != 0)
    {
        auto absInertia = abs(phys_.inertia.y);
        const auto m_inertiaSign = utils::signof(phys_.inertia.y / abs(phys_.inertia.y));
        absInertia = std::max(absInertia - phys_.drag.y, 0.0f);
        phys_.inertia.y = m_inertiaSign * absInertia;
    }

    // Prepare vars for collision detection
    const auto offset = phys_.claimOffset();

    phys_.calculatedOffset = offset;

    // X axis movement handling
    {
        // Moving to the right
        if (offset.x > 0)
            handler.moveRight(offset.x);
        // Moving to the left
        else if (offset.x < 0)
            handler.moveLeft(-offset.x);
    }

    // Y axis movement handling
    {
        // Falling
        if (offset.y > 0)
            handler.moveDown(offset.y);
        // Rising
        else if (offset.y < 0)
            handler.moveUp(-offset.y);
    }

    handler.magnet();
    handler.discoverPosition();

    phys_.appliedOffset.push(trans_.m_pos - oldPos + phys_.pushedOffset);
    phys_.extraoffset = {0.0f, 0.0f};
    phys_.pushedOffset = {0, 0};
}

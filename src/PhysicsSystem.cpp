#include "PhysicsSystem.h"
#include "Core/CoreComponents.h"
#include "Core/Profile.h"
#include <stack>
#include <stdexcept>

const float PhysicsEntityHandler::VerticalOffsetLimitMul = 1.3f;

PhysicsEntityHandler::PhysicsEntityHandler(const CollidersView &cld_, ComponentTransform &trans_, 
        ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, WorldPosition &worldPos_) :
    m_cld{cld_},
    m_trans{trans_},
    m_phys{phys_},
    m_pushbox{phys_.pushbox},
    m_obsFallthrough{obsFallthrough_},
    m_worldPos{worldPos_},
    m_initialPos{trans_.m_pos}
{  
}


AttemptPos::AttemptPos(const Vector2<int> &pos_, bool requireMagnet_, bool haltMovement_, AttemptType attemptType_) :
    pos{pos_},
    requireMagnet{requireMagnet_},
    haltMovement{haltMovement_},
    attemptType{attemptType_}
{}

AttemptPos AttemptPos::addIgnoredObstacle(entt::entity cid_) const
{
    AttemptPos res{*this};
    res.m_ignored.insert(cid_);
    return res;
}

bool AttemptPos::isIgnoringObstacle(entt::entity cid_) const
{
    return m_ignored.contains(cid_);
}

const std::set<entt::entity> &AttemptPos::ignored() const noexcept
{
    return m_ignored;
}

AttemptPos::Key AttemptPos::key() const noexcept
{
    return {haltMovement, pos.x};
}


bool AttemptComparatorRight::operator()(const AttemptPos::Key &lhs_, const AttemptPos::Key &rhs_) const noexcept
{
    return !std::get<0>(lhs_) && std::get<0>(rhs_) || std::get<0>(lhs_) == std::get<0>(rhs_) && std::get<1>(lhs_) > std::get<1>(rhs_);
}


bool AttemptComparatorLeft::operator()(const AttemptPos::Key &lhs_, const AttemptPos::Key &rhs_) const noexcept
{
    return !std::get<0>(lhs_) && std::get<0>(rhs_) || std::get<0>(lhs_) == std::get<0>(rhs_) && std::get<1>(lhs_) < std::get<1>(rhs_);
}


template<typename Cmp>
AttemptContainer<Cmp>::AttemptContainer() :
    m_attempts(Cmp{})
{}

template<typename Cmp>
void AttemptContainer<Cmp>::add(AttemptPos &&attempt_)
{
    m_attempts[attempt_.key()].emplace(std::move(attempt_));
}

template<typename Cmp>
AttemptPos AttemptContainer<Cmp>::extract()
{
    if (m_attempts.empty())
        throw std::logic_error("No attempts left");

    auto first = m_attempts.begin();
    auto res = first->second.top();
    first->second.pop();

    if (first->second.empty())
        m_attempts.erase(first);

    return res;
}

template<typename Cmp>
bool AttemptContainer<Cmp>::empty() const
{
    return m_attempts.empty();
}

template
struct AttemptContainer<AttemptComparatorRight>;

template
struct AttemptContainer<AttemptComparatorLeft>;


void PhysicsEntityHandler::moveRight(const int offset_)
{
    assert(offset_ > 0);

    const auto startingPos = m_trans.m_pos;

    const auto halfWidth = m_pushbox.m_size.x / 2;

    const auto xLoopbackLimit = startingPos.x + 1;

    AttemptContainerRight attempts;
    attempts.add({startingPos.add(offset_, 0), true, false, AttemptType::BACKWARD});

    while (!attempts.empty())
    {
        const auto attempt = attempts.extract();
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

            // Skip ignored obstacle
            if (cld.obstacleType > ObstacleType::NONE && (
                m_obsFallthrough.isIgnoringObstacle(idx) ||
                attempt.isIgnoringObstacle(idx) ||
                cld.obstacleType >= ObstacleType::FLOOR && m_obsFallthrough.isIgnoringAllObstacles()))
                continue;

            const Vector2<int> topPos{attempt.pos.x, highest - 1};
            // Here and below +1 is added because slopes rely on floating point math and that work bad when getting onto the slope
            const bool upCondition = static_cast<float>(startingPos.y - topPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(topPos.x - startingPos.x);
            
            const Vector2<int> bottomPos{attempt.pos.x, cld.m_resolved.bottomY() + m_pushbox.m_size.y};
            const bool downCondition = static_cast<float>(bottomPos.y - startingPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(bottomPos.x - startingPos.x);

            valid = false;
            const auto leftmost = cld.m_resolved.getMostLeftAt(newPb) - halfWidth - 1;
            
            switch (attempt.attemptType)
            {
                case AttemptType::BACKWARD:
                    if (leftmost >= xLoopbackLimit)
                        attempts.add({Vector2{leftmost, attempt.pos.y}, true, true, AttemptType::BACKWARD});
                    if (cld.obstacleType > ObstacleType::MINIMAL)
                        attempts.add({attempt.addIgnoredObstacle(idx)});
                    if (downCondition)
                        attempts.add({bottomPos, true, attempt.haltMovement, AttemptType::DOWNWARD});
                    if (upCondition)
                        attempts.add({topPos, false, attempt.haltMovement, AttemptType::UPWARD});
                    break;

                case AttemptType::UPWARD:
                    if (leftmost >= xLoopbackLimit)
                        attempts.add({Vector2{leftmost, attempt.pos.y}, true, true, AttemptType::UPWARD});
                    if (cld.obstacleType > ObstacleType::MINIMAL)
                        attempts.add({attempt.addIgnoredObstacle(idx)});
                    if (upCondition)
                        attempts.add({topPos, false, attempt.haltMovement, AttemptType::UPWARD});
                    break;

                case AttemptType::DOWNWARD:
                    if (leftmost >= xLoopbackLimit)
                        attempts.add({Vector2{leftmost, attempt.pos.y}, true, true, AttemptType::DOWNWARD});
                    if (cld.obstacleType > ObstacleType::MINIMAL)
                        attempts.add({attempt.addIgnoredObstacle(idx)});
                    if (downCondition)
                        attempts.add({bottomPos, true, attempt.haltMovement, AttemptType::DOWNWARD});
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
            for (const auto &ignored : attempt.ignored())
                m_obsFallthrough.setIgnoreObstacle(ignored);
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

    AttemptContainerLeft attempts;
    attempts.add({startingPos.sub(offset_, 0), true, false, AttemptType::BACKWARD});

    while (!attempts.empty())
    {
        const auto attempt = attempts.extract();
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

            // Skip ignored obstacle
            if (cld.obstacleType > ObstacleType::NONE && (
                m_obsFallthrough.isIgnoringObstacle(idx) ||
                attempt.isIgnoringObstacle(idx) ||
                cld.obstacleType >= ObstacleType::FLOOR && m_obsFallthrough.isIgnoringAllObstacles()))
                continue;

            const Vector2<int> topPos{attempt.pos.x, highest - 1};
            // Here and below +1 is added because slopes rely on floating point math and that work bad when getting onto the slope
            const bool upCondition = static_cast<float>(startingPos.y - topPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(startingPos.x - topPos.x);

            const Vector2<int> bottomPos{attempt.pos.x, cld.m_resolved.bottomY() + m_pushbox.m_size.y};
            const bool downCondition = static_cast<float>(bottomPos.y - startingPos.y) <= 1.0f + VerticalOffsetLimitMul * static_cast<float>(startingPos.x - bottomPos.x);
        
            valid = false;
            const auto rightmost = cld.m_resolved.getMostRightAt(newPb) + halfWidth;
            
            switch (attempt.attemptType)
            {
                case AttemptType::BACKWARD:
                    if (rightmost <= xLoopbackLimit)
                        attempts.add({Vector2{rightmost, attempt.pos.y}, true, true, AttemptType::BACKWARD});
                    if (cld.obstacleType > ObstacleType::MINIMAL)
                        attempts.add({attempt.addIgnoredObstacle(idx)});
                    if (downCondition)
                        attempts.add({bottomPos, true, attempt.haltMovement, AttemptType::DOWNWARD});
                    if (upCondition)
                        attempts.add({topPos, false, attempt.haltMovement, AttemptType::UPWARD});
                    break;

                case AttemptType::UPWARD:
                    if (rightmost <= xLoopbackLimit)
                        attempts.add({Vector2{rightmost, attempt.pos.y}, true, true, AttemptType::UPWARD});
                    if (cld.obstacleType > ObstacleType::MINIMAL)
                        attempts.add({attempt.addIgnoredObstacle(idx)});
                    if (upCondition)
                        attempts.add({topPos, false, attempt.haltMovement, AttemptType::UPWARD});
                    break;

                case AttemptType::DOWNWARD:
                    if (rightmost <= xLoopbackLimit)
                        attempts.add({Vector2{rightmost, attempt.pos.y}, true, true, AttemptType::DOWNWARD});
                    if (cld.obstacleType > ObstacleType::MINIMAL)
                        attempts.add({attempt.addIgnoredObstacle(idx)});
                    if (downCondition)
                        attempts.add({bottomPos, true, attempt.haltMovement, AttemptType::DOWNWARD});
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
            for (const auto &ignored : attempt.ignored())
                m_obsFallthrough.setIgnoreObstacle(ignored);
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

            if (cld.obstacleType > ObstacleType::NONE && (
                m_obsFallthrough.isIgnoringObstacle(idx) ||
                cld.obstacleType >= ObstacleType::FLOOR && m_obsFallthrough.isIgnoringAllObstacles()))
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

            if (cld.obstacleType > ObstacleType::NONE && (
                m_obsFallthrough.isIgnoringObstacle(idx) ||
                cld.obstacleType >= ObstacleType::FLOOR))
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

    const auto realOffset = m_trans.m_pos - m_initialPos;

    const auto pushbox = m_pushbox + m_trans.m_pos;

    for (const auto& [idx, cld] : m_cld.each())
    {
        if (!cld.m_isEnabled)
            continue;

        int highest = m_trans.m_pos.y;
        const auto overlap = cld.m_resolved.checkOverlap(pushbox, highest);

        if ((overlap & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH)
        {
            if (cld.obstacleType > ObstacleType::NONE)
                m_obsFallthrough.setIgnoreObstacle(idx);
        }
        else
        {
            if (m_worldPos.ground.demand)
            {
                if ((overlap & OverlapResult::OVERLAP_X) == OverlapResult::OVERLAP_X
                    && (cld.obstacleType == ObstacleType::NONE ||
                        !m_obsFallthrough.isIgnoringObstacle(idx) &&
                        (cld.obstacleType < ObstacleType::FLOOR || !m_obsFallthrough.isIgnoringAllObstacles())))
                {
                    if (highest - 1 == m_trans.m_pos.y && (m_worldPos.ground.onGround == entt::null || m_worldPos.ground.onSlopeWithAngle != 0.0f))
                    {
                        if (realOffset.y >= 0 || 
                            realOffset.x > 0 && static_cast<float>(realOffset.y) / static_cast<float>(realOffset.x) > cld.m_resolved.topAngleCoef() ||
                            realOffset.x < 0 && static_cast<float>(realOffset.y) / static_cast<float>(realOffset.x) < cld.m_resolved.topAngleCoef())
                        {
                            m_worldPos.ground.onGround = idx;
                            m_worldPos.ground.onSlopeWithAngle = cld.m_resolved.topAngleCoef();
                        }
                        else
                            LOG_WARNING("Grounded but condition is unfulfilled");
                    }
                }
            }

            if (m_worldPos.wall.demand)
            {
                if (cld.obstacleType < ObstacleType::FLOOR)
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
        if (!areaCld_.m_isEnabled)
            continue;

        if (areaCld_.obstacleType > ObstacleType::NONE && (
            m_obsFallthrough.isIgnoringObstacle(idx) ||
            areaCld_.obstacleType >= ObstacleType::FLOOR && m_obsFallthrough.isIgnoringAllObstacles()))
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

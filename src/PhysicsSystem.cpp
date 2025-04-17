#include "PhysicsSystem.h"
#include "Profile.h"

PhysicsSystem::PhysicsSystem(entt::registry &reg_, Vector2<int> levelSize_) :
    m_reg(reg_),
    m_levelSize(levelSize_)
{
}

void PhysicsSystem::prepHitstop()
{
    PROFILE_FUNCTION;

    auto viewPhys = m_reg.view<ComponentPhysical>();
    for (auto [idx, phys] : viewPhys.each())
    {
        if (phys.m_hitstopLeft)
            phys.m_hitstopLeft--;
    }
}

void PhysicsSystem::prepEntities()
{
    PROFILE_FUNCTION;

    auto viewPhys = m_reg.view<ComponentPhysical>();
    for (auto [idx, phys] : viewPhys.each())
    {
        phys.m_onMovingPlatform = false;
    }
}

void PhysicsSystem::updateSMs()
{
    PROFILE_FUNCTION;

    auto viewSM = m_reg.view<StateMachine>();

    for (auto [idx, sm] : viewSM.each())
    {
        if (checkCurrentHitstop(m_reg, idx))
            continue;

        sm.update({&m_reg, idx}, 0);
    }

    /* TODO: notably faster in release build, but harder to debug even with seq, might add debug flags to enable parallel execution
    auto iteratable = viewSM.each();
    std::for_each(std::execution::par, iteratable.begin(), iteratable.end(), [&](auto inp)
    {
        auto [idx, sm] = inp;
        sm.update({&m_reg, idx}, 0);
    });
   */
}

void PhysicsSystem::updatePhysics()
{
    PROFILE_FUNCTION;

    auto viewPhys = m_reg.view<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, PhysicalEvents>();
    auto viewPhysSimplified = m_reg.view<ComponentTransform, ComponentParticlePhysics>();
    const auto viewscld = m_reg.view<ComponentStaticCollider>();

    for (auto [idx, trans, phys, obsfall, ev] : viewPhys.each())
    {
        if (phys.m_hitstopLeft)
            continue;

        proceedEntity(viewscld, idx, trans, phys, obsfall, ev);
    }

    for (auto [idx, trans, phys] : viewPhysSimplified.each())
        proceedEntity(viewscld, trans, phys);

    /* TODO: notably faster in release build, but harder to debug even with seq, might add debug flags to enable parallel execution
    auto iteratable = viewPhys.each();
    std::for_each(std::execution::par, iteratable.begin(), iteratable.end(), [this, &viewscld](auto inp)
    {
        auto [idx, trans, phys, obsfall, ev] = inp;
        proceedEntity(viewscld, trans, phys, obsfall, ev);
    });
    */
}

void PhysicsSystem::updatePhysicalEvents()
{
    PROFILE_FUNCTION;
    
    auto viewPhysEvent = m_reg.view<PhysicalEvents, StateMachine>();

    for (auto [idx, physev, sm] : viewPhysEvent.each())
    {
        if (checkCurrentHitstop(m_reg, idx))
            continue;

        auto *currentState = static_cast<PhysicalState*>(sm.getRealCurrentState());
        if (physev.m_lostGround)
        {
            currentState->onLostGround({&m_reg, idx});
            physev.m_lostGround = false;
        }

        if (physev.m_touchedGround)
        {
            currentState->onTouchedGround({&m_reg, idx});
            physev.m_touchedGround = false;
        }
    }
}

void PhysicsSystem::updateOverlappedObstacles()
{
    PROFILE_FUNCTION;

    auto viewTransPhysObs = m_reg.view<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough>();
    auto viewColliders = m_reg.view<ComponentStaticCollider>();
    for (auto [idx, trans, phys, obsfallthrough] : viewTransPhysObs.each())
    {
        if (phys.m_hitstopLeft)
            continue;
            
        resetEntityObstacles(trans, phys, obsfallthrough, viewColliders);
        obsfallthrough.m_isIgnoringObstacles.update();
    }
}

bool PhysicsSystem::attemptOffsetDown(const auto &clds_, const entt::entity &idx_, const Vector2<float> &originalPos_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, PhysicalEvents &ev_, unsigned int offset_,
    bool noLanding_, float &touchedSlope_, entt::entity &onGround_)
{
    const auto oldPos = originalPos_;
    const auto newPos = originalPos_ + Vector2<int>(0, offset_);

    const auto oldPb = phys_.m_pushbox + oldPos;
    const auto newPb = phys_.m_pushbox + newPos;

    int highest = 0;

    for (auto [idx, cld] : clds_.each())
    {
        if (!cld.m_isEnabled)
            continue;

        auto overlap = cld.m_resolved.checkOverlap(newPb, highest);
        if (checkCollision(overlap, OverlapResult::OVERLAP_BOTH))
        {
            if (cld.m_obstacleId && (!obsFallthrough_.touchedObstacleTop(cld.m_obstacleId) || oldPos.y - highest > 0))
                continue;

            std::cout << "Touched slope top, stopping at top, offset.y > 0\n";

            if (!noLanding_)
            {
                if (cld.m_resolved.m_topAngleCoef != 0)
                    touchedSlope_ = cld.m_resolved.m_topAngleCoef;
                onGround_ = idx;
            }

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;

            return false;
        }
    }

    trans_.m_pos = newPos;

    return true;
}

bool PhysicsSystem::attemptOffsetUp(const auto &clds_, const entt::entity &idx_, const Vector2<float> &originalPos_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, PhysicalEvents &ev_, unsigned int offset_)
{
    const auto oldPos = originalPos_;
    const auto newPos = originalPos_ - Vector2<int>(0, offset_);

    const auto oldPb = phys_.m_pushbox + oldPos;
    const auto newPb = phys_.m_pushbox + newPos;

    for (auto [idx, cld] : clds_.each())
    {
        if (!cld.m_isEnabled)
            continue;

        auto overlap = cld.m_resolved.checkOverlap(newPb);
        if (checkCollision(overlap, OverlapResult::OVERLAP_BOTH))
        {
            if (cld.m_obstacleId && !obsFallthrough_.touchedObstacleBottom(cld.m_obstacleId))
                continue;

            std::cout << "Touched ceiling, stopping at bottom, offset.y < 0\n";

            auto overlapPortion = utils::getOverlapPortion(newPb.getLeftEdge(), newPb.getRightEdge(), cld.m_resolved.m_points[0].x, cld.m_resolved.m_points[1].x);

            if (overlapPortion >= 0.7f)
            {
                phys_.m_velocity.y = 0;
                phys_.m_inertia.y = 0;
            }

            return false;
        }
    }

    trans_.m_pos = newPos;

    return true;
}

bool PhysicsSystem::attemptOffsetHorizontal(const auto &clds_, const entt::entity &idx_, const Vector2<float> &originalPos_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, PhysicalEvents &ev_, int offset_,
    int originalY_, int maxYOffset_, int naturalYOffset_, float &touchedSlope_, entt::entity &onGround_)
{
    const auto oldPos = trans_.m_pos;
    const auto newPos = trans_.m_pos + Vector2<int>(offset_, 0);

    const auto oldPb = phys_.m_pushbox + oldPos;
    const auto newPb = phys_.m_pushbox + newPos;

    bool mustRise = false;
    int completeHighest = std::numeric_limits<int>::max();

    entt::entity onGround = entt::null;
    float touchedSlope = 0.0f;

    for (auto [idx, cld] : clds_.each())
    {
        if (!cld.m_isEnabled)
            continue;

        int highest = std::numeric_limits<int>::max();

        auto overlap = cld.m_resolved.checkOverlap(newPb, highest);

        // If we touched collider
        if (checkCollision(overlap, OverlapResult::OVERLAP_BOTH))
        {
            highest--;

            // If we can rise on top of it
            if (originalY_ - highest <= maxYOffset_)
            {
                if (cld.m_obstacleId && !obsFallthrough_.touchedObstacleSlope(cld.m_obstacleId))
                    continue;

                std::cout << "Touched slope, (potentially) getting at it's top, offset.x > 0\n";

                if (!mustRise || highest < completeHighest)
                {
                    completeHighest = highest;
                    mustRise = true;
                    onGround = idx;
                    touchedSlope = cld.m_resolved.m_topAngleCoef;
                }
            }
            // If its an actual wall
            else
            {
                if (cld.m_obstacleId && !obsFallthrough_.touchedObstacleSide(cld.m_obstacleId))
                    continue;

                auto overlapPortion = utils::getOverlapPortion(newPb.getTopEdge(), newPb.getBottomEdge(), highest, cld.m_resolved.m_points[2].y);

                if (overlapPortion >= 0.15)
                {
                    std::cout << "Touched edge, stopping at it (hard), offset.x > 0\n";
                    if (phys_.m_velocity.x + phys_.m_inertia.x > 0)
                    {
                        phys_.m_velocity.x = 0;
                        phys_.m_inertia.x = 0;
                    }
                }
                else
                    std::cout << "Touched edge, stopping at it (soft), offset.x > 0\n";

                return false;
            }
        }
    }

    if (mustRise)
    {
        if (attemptOffsetUp(clds_, idx_, newPos, trans_, phys_, obsFallthrough_, ev_, newPos.y - completeHighest))
        {
            if (naturalYOffset_ >= 0)
            {
                if (touchedSlope != 0)
                    touchedSlope_ = touchedSlope;
                onGround_ = onGround;
            }

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
    }
    else
    {
        trans_.m_pos = newPos;
    }

    return true;
}

void PhysicsSystem::proceedEntity(const auto &clds_, const entt::entity &idx_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, PhysicalEvents &ev_)
{
    const auto oldPos = trans_.m_pos;

    // Common stuff
    phys_.m_velocity += phys_.m_gravity;

    if (phys_.m_inertia.x != 0)
    {
        auto absInertia = abs(phys_.m_inertia.x);
        const auto m_inertiaSign = utils::signof(phys_.m_inertia.x / abs(phys_.m_inertia.x));
        absInertia = std::max(absInertia - phys_.m_drag.x, 0.0f);
        phys_.m_inertia.x = m_inertiaSign * absInertia;
    }

    if (phys_.m_inertia.y != 0)
    {
        auto absInertia = abs(phys_.m_inertia.y);
        const auto m_inertiaSign = utils::signof(phys_.m_inertia.y / abs(phys_.m_inertia.y));
        absInertia = std::max(absInertia - phys_.m_drag.y, 0.0f);
        phys_.m_inertia.y = m_inertiaSign * absInertia;
    }

    //std::cout << m_staticColliderQuery.size() << std::endl;

    // Prepare vars for collision detection
    auto pb = phys_.m_pushbox + trans_.m_pos;
    auto offset = phys_.claimOffset();

    if (phys_.m_mulInsidePushbox && isInsidePushbox(pb, idx_))
    {
        offset = offset.mulComponents(*phys_.m_mulInsidePushbox);
    }

    phys_.m_calculatedOffset = offset;
    bool noLanding = phys_.m_noLanding;

    const auto oldHeight = trans_.m_pos.y;
    const auto oldTop = pb.getTopEdge();
    const auto oldRightEdge = pb.getRightEdge();
    const auto oldLeftEdge = pb.getLeftEdge();

    entt::entity onGround = entt::null;
    float touchedSlope = 0.0f;
    int highest = m_levelSize.y;

    // X axis movement handling
    {
        int originalY = trans_.m_pos.y;
        int maxOffsetY = 1.3f * abs(offset.x);

        // Moving to the right
        if (offset.x > 0)
        {
            for (int i = 0; i < offset.x; ++i)
                if (!attemptOffsetHorizontal(clds_, idx_, trans_.m_pos, trans_, phys_, obsFallthrough_, ev_, 1, originalY, maxOffsetY, offset.y, touchedSlope, onGround))
                    break;
        }
        // Moving to the left
        else if (offset.x < 0)
        {
            for (int i = 0; i > offset.x; --i)
                if (!attemptOffsetHorizontal(clds_, idx_, trans_.m_pos, trans_, phys_, obsFallthrough_, ev_, -1, originalY, maxOffsetY, offset.y, touchedSlope, onGround))
                    break;
        }
    }

    // Y axis movement handling
    {
        // Falling / staying in place
        if (offset.y >= 0)
        {
            for (int i = 0; i < offset.y; ++i)
                if (!attemptOffsetDown(clds_, idx_, trans_.m_pos, trans_, phys_, obsFallthrough_, ev_, 1, noLanding, touchedSlope, onGround))
                    break;

            pb = phys_.m_pushbox + trans_.m_pos;
        }
        // Rising
        else
        {
            if (noLanding)
            {
                touchedSlope = 0;
                onGround = entt::null;
            }
            
            for (int i = 0; i > offset.y; --i)
                if (!attemptOffsetUp(clds_, idx_, trans_.m_pos, trans_, phys_, obsFallthrough_, ev_, 1))
                    break;

            pb = phys_.m_pushbox + trans_.m_pos;
        }
    }

    phys_.m_lastSlopeAngle = phys_.m_onSlopeWithAngle;
    phys_.m_onSlopeWithAngle = touchedSlope;

    if (onGround != entt::null)
    {
        phys_.m_onGround = onGround;
        ev_.m_touchedGround = true;
    }
    else
    {
        if (offset.y < 0 || !magnetEntity(clds_, trans_, phys_, obsFallthrough_))
        {
            phys_.m_onGround = entt::null;
            ev_.m_lostGround = true;
        }
    }

    phys_.m_appliedOffset = trans_.m_pos - oldPos + phys_.m_pushedOffset;
    phys_.m_extraoffset = {0.0f, 0.0f};
    phys_.m_pushedOffset = {0, 0};
}

void PhysicsSystem::proceedEntity(const auto &clds_, ComponentTransform &trans_, ComponentParticlePhysics &phys_)
{
    // Common stuff
    phys_.m_velocity += phys_.m_gravity;

    phys_.applyDrag();

    // Prepare vars for collision detection
    const auto offset = phys_.claimOffset();
    trans_.m_pos += offset;
}

bool PhysicsSystem::magnetEntity(const auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, const ComponentObstacleFallthrough &obsFallthrough_)
{
    //std::cout << "Magnet " << rand() << std::endl;
    if (phys_.m_magnetLimit <= 0)
        return false;

    auto pb = phys_.m_pushbox + trans_.m_pos;
    auto bot = pb.getBottomEdge();

    int height = trans_.m_pos.y;
    const auto [found, pcld] = getHighestVerticalMagnetCoord(clds_, pb, height, obsFallthrough_.m_ignoredObstacles, obsFallthrough_.m_isIgnoringObstacles.isActive());
    if ( found != entt::null )
    {
        int magnetRange = height - trans_.m_pos.y;
        if (magnetRange <= phys_.m_magnetLimit)
        {
            //std::cout << "MAGNET: " << magnetRange << std::endl;
            trans_.m_pos.y = height - 1;
            phys_.m_lastSlopeAngle = phys_.m_onSlopeWithAngle;
            phys_.m_onSlopeWithAngle = (bot > pcld->m_highestSlopePoint ? pcld->m_topAngleCoef : 0.0f);
            phys_.m_onGround = found;
            return true;
        }
    }

    return false;
}

std::pair<entt::entity, const SlopeCollider*> PhysicsSystem::getHighestVerticalMagnetCoord(const auto &clds_, const Collider &cld_, int &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_)
{
    auto baseCoord = coord_;
    auto bot = cld_.getBottomEdge();
    entt::entity foundGround = entt::null;
    const SlopeCollider *foundcld = nullptr;
    
    for (const auto [idx, areaCld_] : clds_.each())
    {
        if (!areaCld_.m_isEnabled || areaCld_.m_obstacleId && (ignoreAllObstacles_ || ignoredObstacles_.contains(areaCld_.m_obstacleId)))
            continue;

        int height = 0;
        auto horOverlap = areaCld_.m_resolved.checkOverlap(cld_, height);
        if (checkCollision(horOverlap, OverlapResult::OVERLAP_X))
        {
            if (height >= baseCoord && (foundGround == entt::null || height < coord_))
            {
                coord_ = height;
                foundGround = idx;
                foundcld = &areaCld_.m_resolved;
            }
        }
    }

    return {foundGround, foundcld};
}

bool PhysicsSystem::isInsidePushbox(const Collider &pb_, const entt::entity &idx_)
{
    auto viewPhys = m_reg.view<ComponentTransform, ComponentPhysical>();

    for (auto [idx, trans, phys] : viewPhys.each())
    {
        if (idx == idx_)
            continue;
        
        if (phys.m_pushbox.m_size.x <= 0 || phys.m_pushbox.m_size.y <= 0)
            continue;

        auto pb2 = phys.m_pushbox + trans.m_pos;
        auto overlap = pb_.checkOverlap(pb2);

        if (checkCollision(overlap, OverlapResult::OVERLAP_BOTH))
            return true;
    }

    return false;
}

void PhysicsSystem::resetEntityObstacles(const ComponentTransform &trans_, const ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, const auto &clds_)
{
    updateTouchedObstacles(phys_.m_pushbox + trans_.m_pos, obsFallthrough_, clds_);
    std::set<int> res;
    std::set_intersection(
        obsFallthrough_.m_ignoredObstacles.begin(), obsFallthrough_.m_ignoredObstacles.end(),
        obsFallthrough_.m_overlappedObstacles.begin(), obsFallthrough_.m_overlappedObstacles.end(),
        std::inserter(res, res.begin()));

    //if (obsFallthrough_.m_ignoredObstacles.size() != res.size())
    //    std::cout << "Reseted obstacles\n";

    obsFallthrough_.m_ignoredObstacles = res;
}

void PhysicsSystem::updateTouchedObstacles(const Collider &pb_, ComponentObstacleFallthrough &obsFallthrough_, const auto &clds_)
{
    obsFallthrough_.m_overlappedObstacles.clear();

    for (auto [idx, cld] : clds_.each())
    {
        if (obsFallthrough_.m_overlappedObstacles.contains(cld.m_obstacleId))
            continue;

        if (checkCollision(cld.m_resolved.checkOverlap(pb_), OverlapResult::OVERLAP_BOTH))
            obsFallthrough_.m_overlappedObstacles.insert(cld.m_obstacleId);
    }
}
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

void PhysicsSystem::proceedEntity(const auto &clds_, const entt::entity &idx_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, PhysicalEvents &ev_)
{
    const auto oldPos = trans_.m_pos;
    auto oldPosXOnly = trans_.m_pos; // Set once X offset is applied

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

    // Fall collision detection - single collider vs single entity
    auto resolveFall = [&](entt::entity idx_, const ComponentStaticCollider &csc_)
    {
        if (!csc_.m_isEnabled)
            return;

        auto overlap = csc_.m_resolved.checkOverlap(pb, highest);
        if (checkCollision(overlap, OverlapResult::OVERLAP_BOTH))
        {
            if (csc_.m_obstacleId && (!obsFallthrough_.touchedObstacleTop(csc_.m_obstacleId) || oldPosXOnly.y - highest > 0))
                return;

            std::cout << "Touched slope top, teleporting on top, offset.y > 0\n";

            trans_.m_pos.y = highest - 1;

            if (!noLanding)
            {
                if (csc_.m_resolved.m_topAngleCoef != 0)
                    touchedSlope = csc_.m_resolved.m_topAngleCoef;
                onGround = idx_;
            }

            pb = phys_.m_pushbox + trans_.m_pos;

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
    };

    // Rise collision detection
    auto resolveRise = [&](const ComponentStaticCollider &csc_)
    {
        if (!csc_.m_isEnabled)
            return;

        auto overlap = csc_.m_resolved.checkOverlap(pb, highest);
        if (checkCollision(overlap, OverlapResult::OVERLAP_BOTH))
        {
            if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleBottom(csc_.m_obstacleId))
                return;

            std::cout << "Touched ceiling, teleporting to bottom, offset.y < 0\n";

            auto overlapPortion = utils::getOverlapPortion(pb.getLeftEdge(), pb.getRightEdge(), csc_.m_resolved.m_points[0].x, csc_.m_resolved.m_points[1].x);

            if (overlapPortion >= 0.7f)
            {
                phys_.m_velocity.y = 0;
                phys_.m_inertia.y = 0;
            }

            trans_.m_pos.y = csc_.m_resolved.m_points[2].y + pb.m_size.y;
            pb = phys_.m_pushbox + trans_.m_pos;
        }
    };
    
    // Movement to right collision detection
    auto resolveRight = [&](entt::entity idx_, const ComponentStaticCollider &csc_)
    {
        if (!csc_.m_isEnabled)
            return;

        auto overlap = csc_.m_resolved.checkOverlap(pb, highest);

        // If we touched collider
        if (checkCollision(overlap, OverlapResult::OVERLAP_BOTH))
        {
            // If we can rise on top of it
            if (oldPos.y - highest <= (1.3f * abs(trans_.m_pos.x - oldPos.x)))
            {
                if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleSlope(csc_.m_obstacleId))
                    return;

                std::cout << "Touched slope, teleporting on top, offset.x > 0\n";

                trans_.m_pos.y = highest - 1;
                pb = phys_.m_pushbox + trans_.m_pos;

                if (offset.y >= 0)
                {
                    if (csc_.m_resolved.m_topAngleCoef != 0)
                        touchedSlope = csc_.m_resolved.m_topAngleCoef;
                    onGround = idx_;
                }

                if (phys_.m_velocity.y > 0)
                    phys_.m_velocity.y = 0;
                if (phys_.m_inertia.y > 0)
                    phys_.m_inertia.y = 0;
            }
            // If its an actual wall
            else
            {
                if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleSide(csc_.m_obstacleId))
                    return;

                auto overlapPortion = utils::getOverlapPortion(pb.getTopEdge(), pb.getBottomEdge(), highest, csc_.m_resolved.m_points[2].y);

                if (overlapPortion >= 0.1 || !phys_.m_onSlopeWithAngle != 0)
                {
                    std::cout << "Touched edge, teleporting to it, offset.x > 0\n";
                    trans_.m_pos.x = csc_.m_resolved.m_tlPos.x - pb.m_size.x / 2 - 1;
                    pb = phys_.m_pushbox + trans_.m_pos;
                }

                if (overlapPortion >= 0.15)
                {
                    //std::cout << "Hard collision, limiting speed\n";
                    if (phys_.m_velocity.x + phys_.m_inertia.x > 0)
                    {
                        phys_.m_velocity.x = 0;
                        phys_.m_inertia.x = 0;
                    }
                }
            }
        }
    };

    // Movement to left collision detection
    auto resolveLeft = [&](entt::entity idx_, const ComponentStaticCollider &csc_)
    {
        if (!csc_.m_isEnabled)
            return;

        auto overlap = csc_.m_resolved.checkOverlap(pb, highest);

        // If we touched collider
        if (checkCollision(overlap, OverlapResult::OVERLAP_BOTH))
        {
            // If we can rise on top of it
            if (oldPos.y - highest <= (1.3f * abs(trans_.m_pos.x - oldPos.x)))
            {
                if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleSlope(csc_.m_obstacleId))
                    return;

                std::cout << "Touched slope, teleporting on top, offset.x < 0\n";

                trans_.m_pos.y = highest - 1;
                pb = phys_.m_pushbox + trans_.m_pos;

                if (offset.y >= 0)
                {
                    if (csc_.m_resolved.m_topAngleCoef != 0)
                        touchedSlope = csc_.m_resolved.m_topAngleCoef;
                    onGround = idx_;
                }

                if (phys_.m_velocity.y > 0)
                    phys_.m_velocity.y = 0;
                if (phys_.m_inertia.y > 0)
                    phys_.m_inertia.y = 0;
            }
            // If its an actual wall
            else
            {
                if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleSide(csc_.m_obstacleId))
                    return;

                auto overlapPortion = utils::getOverlapPortion(pb.getTopEdge(), pb.getBottomEdge(), highest, csc_.m_resolved.m_points[2].y);

                if (overlapPortion >= 0.1 || !phys_.m_onSlopeWithAngle != 0)
                {
                    std::cout << "Touched edge, teleporting to it, offset.x < 0\n";
                    trans_.m_pos.x = csc_.m_resolved.m_points[1].x + pb.m_size.x / 2;
                    pb = phys_.m_pushbox + trans_.m_pos;
                }

                if (overlapPortion >= 0.15)
                {
                    //std::cout << "Hard collision, limiting speed\n";
                    if (phys_.m_velocity.x + phys_.m_inertia.x < 0)
                    {
                        phys_.m_velocity.x = 0;
                        phys_.m_inertia.x = 0;
                    }
                }
            }
        }
    };

    // X axis movement handling
    {
        trans_.m_pos.x += offset.x;
        pb = phys_.m_pushbox + trans_.m_pos;

        // Moving to the right

        if (offset.x > 0)
        {
            clds_.each(resolveRight); // NOTE: foreach loops are slightly faster, maybe its worth it to move lambdas code here
        }
        // Moving to the left
        else if (offset.x < 0)
        {
            clds_.each(resolveLeft);
        }
    }

    // Y axis movement handling
    {
        oldPosXOnly = trans_.m_pos;
        trans_.m_pos.y += offset.y;
        pb = phys_.m_pushbox + trans_.m_pos;

        // Falling / staying in place
        if (offset.y >= 0)
        {
            clds_.each(resolveFall);
        }
        // Rising
        else
        {
            if (noLanding)
            {
                touchedSlope = 0;
                onGround = entt::null;
            }

            clds_.each(resolveRise);
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
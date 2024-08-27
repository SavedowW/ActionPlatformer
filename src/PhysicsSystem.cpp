#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem(entt::registry &reg_, Vector2<float> levelSize_) :
    m_reg(reg_),
    m_levelSize(levelSize_)
{
}

void PhysicsSystem::prepEntities()
{
    auto viewPhys = m_reg.view<ComponentPhysical>();
    for (auto [idx, phys] : viewPhys.each())
    {
        phys.m_onMovingPlatform = false;
    }
}

void PhysicsSystem::updateSMs()
{
    auto viewSM = m_reg.view<StateMachine>();

    for (auto [idx, sm] : viewSM.each())
    {
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
    auto viewPhys = m_reg.view<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, PhysicalEvents>();
    auto viewPhysSimplified = m_reg.view<ComponentTransform, ComponentParticlePhysics>();
    const auto viewscld = m_reg.view<ComponentStaticCollider>();

    for (auto [idx, trans, phys, obsfall, ev] : viewPhys.each())
        proceedEntity(viewscld, trans, phys, obsfall, ev);

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
    auto viewPhysEvent = m_reg.view<PhysicalEvents, StateMachine>();

    for (auto [idx, physev, sm] : viewPhysEvent.each())
    {
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
    auto viewTransPhysObs = m_reg.view<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough>();
    auto viewColliders = m_reg.view<ComponentStaticCollider>();
    for (auto [idx, trans, phys, obsfallthrough] : viewTransPhysObs.each())
    {
        resetEntityObstacles(trans, phys, obsfallthrough, viewColliders);
        obsfallthrough.m_isIgnoringObstacles.update();
    }
}

// TODO: Revisit collision detection
// Current collision system would make sense if the game used ints to describe colliders
// However, as it is now, we cant meaningfully compare edges, plus there is a lot of edge cases
// Its better to have a simple overlap check and instead use delta (was above on previous frame and now is below, etc)

void PhysicsSystem::proceedEntity(const auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, PhysicalEvents &ev_)
{
    auto oldPos = trans_.m_pos;

    // Common stuff
    phys_.m_velocity += phys_.m_gravity;

    if (phys_.m_inertia.x != 0)
    {
        auto absInertia = abs(phys_.m_inertia.x);
        auto m_inertiaSign = utils::signof(phys_.m_inertia.x / abs(phys_.m_inertia.x));
        absInertia = std::max(absInertia - phys_.m_drag.x, 0.0f);
        phys_.m_inertia.x = m_inertiaSign * absInertia;
    }

    if (phys_.m_inertia.y != 0)
    {
        auto absInertia = abs(phys_.m_inertia.y);
        auto m_inertiaSign = utils::signof(phys_.m_inertia.y / abs(phys_.m_inertia.y));
        absInertia = std::max(absInertia - phys_.m_drag.y, 0.0f);
        phys_.m_inertia.y = m_inertiaSign * absInertia;
    }

    //std::cout << m_staticColliderQuery.size() << std::endl;

    // Prepare vars for collision detection
    auto offset = phys_.getPosOffest();
    phys_.m_calculatedOffset = offset;
    auto pb = phys_.m_pushbox + trans_.m_pos;
    bool noUpwardLanding = phys_.m_noUpwardLanding;

    auto oldHeight = trans_.m_pos.y;
    auto oldTop = pb.getTopEdge();
    auto oldRightEdge = pb.getRightEdge();
    auto oldLeftEdge = pb.getLeftEdge();

    bool groundCollision = false;
    float touchedSlope = 0.0f;
    float highest = m_levelSize.y;

    // Fall collision detection - single collider vs single entity
    auto resolveFall = [&](const ComponentStaticCollider &csc_)
    {
        if (!csc_.m_isEnabled || oldTop >= csc_.m_collider.m_points[2].y)
            return;
        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        if ((overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y))
        {
            if (csc_.m_obstacleId && (!obsFallthrough_.touchedObstacleTop(csc_.m_obstacleId) || oldHeight - highest > abs(trans_.m_pos.x - oldPos.x)) || oldHeight - highest >= 0.1f + abs(csc_.m_collider.m_topAngleCoef * (trans_.m_pos.x - oldPos.x)))
                return;

            //std::cout << "Touched slope top, teleporting on top, offset.y > 0\n";

            trans_.m_pos.y = highest;
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = (pb.getBottomEdge() > csc_.m_collider.m_highestSlopePoint ? csc_.m_collider.m_topAngleCoef : 0.0f);
            groundCollision = true;
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
        if (!csc_.m_isEnabled || csc_.m_collider.m_highestSlopePoint > oldTop)
            return;

        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        if ((overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y))
        {
            if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleBottom(csc_.m_obstacleId))
                return;

            //std::cout << "Touched ceiling, teleporting to bottom, offset.y < 0\n";

            auto overlapPortion = utils::getOverlapPortion(pb.getLeftEdge(), pb.getRightEdge(), csc_.m_collider.m_points[0].x, csc_.m_collider.m_points[1].x);

            if (overlapPortion >= 0.7f)
            {
                phys_.m_velocity.y = 0;
                phys_.m_inertia.y = 0;
            }

            trans_.m_pos.y = csc_.m_collider.m_points[2].y + pb.getSize().y;
            pb = phys_.m_pushbox + trans_.m_pos;
        }
    };
    
    // Movement to right collision detection
    auto resolveRight = [&](const ComponentStaticCollider &csc_)
    {
        if (!csc_.m_isEnabled)
            return;

        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        bool aligned = csc_.m_collider.getOrientationDir() > 0;

        if ((overlap & (utils::OverlapResult::TOUCH_MIN1_MAX2 << 6)) && (overlap & utils::OverlapResult::BOTH_OOT))
        {
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = (pb.getBottomEdge() > csc_.m_collider.m_highestSlopePoint ? csc_.m_collider.m_topAngleCoef : 0.0f);
            groundCollision = true;
        }

        if (aligned && (overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y) && abs(highest - oldPos.y) <= 1.3f * abs(trans_.m_pos.x - oldPos.x)) // Touched slope from right direction
        {
            if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleSlope(csc_.m_obstacleId))
                return;

            //std::cout << "Touched slope, teleporting on top, offset.x > 0\n";

            trans_.m_pos.y = highest;
            pb = phys_.m_pushbox + trans_.m_pos;
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = (pb.getBottomEdge() > csc_.m_collider.m_highestSlopePoint ? csc_.m_collider.m_topAngleCoef : 0.0f);
            groundCollision = true;

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
        else if ((overlap & utils::OverlapResult::OOT_X) && (overlap & utils::OverlapResult::OVERLAP_Y)) // Touched as wall
        {
            if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleSide(csc_.m_obstacleId))
                return;

            auto overlapPortion = utils::getOverlapPortion(pb.getTopEdge(), pb.getBottomEdge(), highest, csc_.m_collider.m_points[2].y);

            if (overlapPortion >= 0.1 || !phys_.m_onSlopeWithAngle != 0)
            {
                //std::cout << "Touched edge, teleporting to it, offset.x > 0\n";
                trans_.m_pos.x = csc_.m_collider.m_tlPos.x - pb.m_halfSize.x;
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
    };

    // Movement to left collision detection
    auto resolveLeft = [&](const ComponentStaticCollider &csc_)
    {
        if (!csc_.m_isEnabled)
            return;

        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        bool aligned = csc_.m_collider.getOrientationDir() < 0;

        if ((overlap & (utils::OverlapResult::TOUCH_MIN1_MAX2 << 6)) && (overlap & utils::OverlapResult::BOTH_OOT))
        {
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = (pb.getBottomEdge() > csc_.m_collider.m_highestSlopePoint ? csc_.m_collider.m_topAngleCoef : 0.0f);
            groundCollision = true;
        }

        if (aligned && (overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y) && abs(highest - oldPos.y) <= 1.3f * abs(trans_.m_pos.x - oldPos.x)) // Touched slope from right direction
        {
            if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleSlope(csc_.m_obstacleId))
                return;

            //std::cout << "Touched slope, teleporting on top, offset.x < 0\n";

            trans_.m_pos.y = highest;
            pb = phys_.m_pushbox + trans_.m_pos;
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = (pb.getBottomEdge() > csc_.m_collider.m_highestSlopePoint ? csc_.m_collider.m_topAngleCoef : 0.0f);
            groundCollision = true;

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
        else if ((overlap & utils::OverlapResult::OOT_X) && (overlap & utils::OverlapResult::OVERLAP_Y)) // Touched as wall
        {
            if (csc_.m_obstacleId && !obsFallthrough_.touchedObstacleSide(csc_.m_obstacleId))
                return;

            auto overlapPortion = utils::getOverlapPortion(pb.getTopEdge(), pb.getBottomEdge(), highest, csc_.m_collider.m_points[2].y);

            if (overlapPortion >= 0.1 || !phys_.m_onSlopeWithAngle != 0)
            {
                //std::cout << "Touched edge, teleporting to it, offset.x < 0\n";
                trans_.m_pos.x = csc_.m_collider.m_tlPos.x + csc_.m_collider.m_size.x + pb.m_halfSize.x;
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
            if (noUpwardLanding)
            {
                touchedSlope = 0;
                groundCollision = false;
            }

            clds_.each(resolveRise);
        }
    }

    phys_.m_lastSlopeAngle = phys_.m_onSlopeWithAngle;
    phys_.m_onSlopeWithAngle = touchedSlope;

    if (groundCollision)
    {
        ev_.m_touchedGround = true;
    }
    else
    {
        if (!magnetEntity(clds_, trans_, phys_, obsFallthrough_))
            ev_.m_lostGround = true;
    }

    phys_.m_appliedOffset = trans_.m_pos - oldPos;
    phys_.m_extraoffset = {0.0f, 0.0f};
}

void PhysicsSystem::proceedEntity(const auto &clds_, ComponentTransform &trans_, ComponentParticlePhysics &phys_)
{
    // Common stuff
    phys_.m_velocity += phys_.m_gravity;

    phys_.applyDrag();

    // Prepare vars for collision detection
    auto offset = phys_.getPosOffest();
    trans_.m_pos += offset;
}

bool PhysicsSystem::magnetEntity(const auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, const ComponentObstacleFallthrough &obsFallthrough_)
{
    if (phys_.m_magnetLimit <= 0.0f)
        return false;

    auto pb = phys_.m_pushbox + trans_.m_pos;
    auto bot = pb.getBottomEdge();

    float height = trans_.m_pos.y;
    const auto [found, pcld] = getHighestVerticalMagnetCoord(clds_, pb, height, obsFallthrough_.m_ignoredObstacles, obsFallthrough_.m_isIgnoringObstacles.isActive());
    if ( found )
    {
        float magnetRange = height - trans_.m_pos.y;
        if (magnetRange <= phys_.m_magnetLimit)
        {
            //std::cout << "MAGNET: " << magnetRange << std::endl;
            trans_.m_pos.y = height;
            phys_.m_lastSlopeAngle = phys_.m_onSlopeWithAngle;
            phys_.m_onSlopeWithAngle = (bot > pcld->m_highestSlopePoint ? pcld->m_topAngleCoef : 0.0f);
            return true;
        }
    }

    return false;
}

std::pair<bool, const SlopeCollider*> PhysicsSystem::getHighestVerticalMagnetCoord(const auto &clds_, const Collider &cld_, float &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_)
{
    float baseCoord = coord_;
    float bot = cld_.getBottomEdge();
    bool isFound = false;
    const SlopeCollider *foundcld = nullptr;
    
    for (const auto [idx, areaCld_] : clds_.each())
    {
        if (!areaCld_.m_isEnabled || areaCld_.m_obstacleId && (ignoreAllObstacles_ || ignoredObstacles_.contains(areaCld_.m_obstacleId)))
            continue;

        auto horOverlap = utils::getOverlap<0>(areaCld_.m_collider.m_points[0].x, areaCld_.m_collider.m_points[1].x, cld_.getLeftEdge(), cld_.getRightEdge()); //cld.getHorizontalOverlap(cld_);
        if (!!(horOverlap & utils::OverlapResult::OVERLAP_X))
        {
            auto height = areaCld_.m_collider.getTopHeight(cld_, horOverlap);
            if (height > baseCoord && (!isFound || height < coord_))
            {
                coord_ = height;
                isFound = true;
                foundcld = &areaCld_.m_collider;
            }
        }
    }

    return {isFound, foundcld};
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

    float dumped = 0.0f;

    for (auto [idx, cld] : clds_.each())
    {
        if (obsFallthrough_.m_overlappedObstacles.contains(cld.m_obstacleId))
            continue;

        if (!!(cld.m_collider.getFullCollisionWith(pb_, dumped) & utils::OverlapResult::BOTH_OOT))
            obsFallthrough_.m_overlappedObstacles.insert(cld.m_obstacleId);
    }
}
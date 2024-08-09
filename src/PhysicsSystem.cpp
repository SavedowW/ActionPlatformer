#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem(entt::registry &reg_, Vector2<float> levelSize_) :
    m_reg(reg_),
    m_levelSize(levelSize_)
{
}

void PhysicsSystem::update()
{
    //m_physicalQuery{reg_.makeQuery<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough>()},
    //m_staticColliderQuery{reg_.makeQuery<ComponentStaticCollider>()},
    auto viewPhys = m_reg.view<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough>();
    auto viewscld = m_reg.view<ComponentStaticCollider>();

    for (auto [idx, trans, phys, obsfall] : viewPhys.each())
    {
        if (m_reg.all_of<StateMachine>(idx))
            proceedEntity(viewscld, idx, trans, phys, obsfall, &m_reg.get<StateMachine>(idx));
        else
            proceedEntity(viewscld, idx, trans, phys, obsfall, nullptr);
    }
}

void PhysicsSystem::proceedEntity(auto &clds_, entt::entity idx_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, StateMachine *sm_)
{
    auto oldPos = trans_.m_pos;

    // Common stuff
    phys_.m_velocity += phys_.m_gravity;

    if (phys_.m_inertia.x != 0)
    {
        auto absInertia = abs(phys_.m_inertia.x);
        auto m_inertiaSign = phys_.m_inertia.x / abs(phys_.m_inertia.x);
        absInertia = std::max(absInertia - phys_.m_drag.x, 0.0f);
        phys_.m_inertia.x = m_inertiaSign * absInertia;
    }

    if (phys_.m_inertia.y != 0)
    {
        auto absInertia = abs(phys_.m_inertia.y);
        auto m_inertiaSign = phys_.m_inertia.y / abs(phys_.m_inertia.y);
        absInertia = std::max(absInertia - phys_.m_drag.y, 0.0f);
        phys_.m_inertia.y = m_inertiaSign * absInertia;
    }

    //std::cout << m_staticColliderQuery.size() << std::endl;

    // Prepare vars for collision detection
    auto offset = phys_.getPosOffest();
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
    auto resolveFall = [&](const ComponentStaticCollider &csc_, int obstacleId_)
    {
        if (oldTop >= csc_.m_collider.m_points[2].y)
            return;
        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        if ((overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y))
        {
            if (obstacleId_ && (!obsFallthrough_.touchedObstacleTop(obstacleId_) || highest < oldHeight))
                return;

            //std::cout << "Touched slope top, teleporting on top, offset.y > 0\n";

            trans_.m_pos.y = highest;
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            groundCollision = true;
            pb = phys_.m_pushbox + trans_.m_pos;

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
    };

    // Rise collision detection
    auto resolveRise = [&](const ComponentStaticCollider &csc_, int obstacleId_)
    {
        if (csc_.m_collider.m_highestSlopePoint > oldTop)
            return;

        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        if ((overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y))
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleBottom(obstacleId_))
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
    auto resolveRight = [&](const ComponentStaticCollider &csc_, int obstacleId_)
    {
        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        bool aligned = csc_.m_collider.getOrientationDir() > 0;
        auto heightDiff = abs(csc_.m_collider.m_lowestSlopePoint - pb.getBottomEdge());

        if ((overlap & utils::OverlapResult::TOUCH_MIN1_MAX2) && (overlap & utils::OverlapResult::BOTH_OOT))
        {
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            groundCollision = true;
        }

        if (aligned && pb.getTopEdge() <= csc_.m_collider.m_lowestSlopePoint && (overlap & utils::OverlapResult::OOT_SLOPE) && (overlap & utils::OverlapResult::OVERLAP_Y)) // Touched slope from right direction
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleSlope(obstacleId_))
                return;

            //std::cout << "Touched slope, teleporting on top, offset.x > 0\n";

            trans_.m_pos.y = highest;
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            pb = phys_.m_pushbox + trans_.m_pos;
            groundCollision = true;

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
        else if ((!aligned || csc_.m_collider.m_hasBox) && (overlap & utils::OverlapResult::OOT_BOX) && (overlap & utils::OverlapResult::OVERLAP_Y) && pb.getBottomEdge() > csc_.m_collider.m_lowestSlopePoint) // Touched inner box
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleSide(obstacleId_))
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
                if (phys_.m_velocity.x > 0)
                    phys_.m_velocity.x = 0;
                if (phys_.m_inertia.x > 0)
                    phys_.m_inertia.x = 0;
            }
        }
    };

    // Movement to left collision detection
    auto resolveLeft = [&](const ComponentStaticCollider &csc_, int obstacleId_)
    {
        auto overlap = csc_.m_collider.getFullCollisionWith(pb, highest);
        bool aligned = csc_.m_collider.getOrientationDir() < 0;
        auto heightDiff = abs(csc_.m_collider.m_lowestSlopePoint - pb.getBottomEdge());

        if ((overlap & utils::OverlapResult::TOUCH_MIN1_MAX2) && (overlap & utils::OverlapResult::BOTH_OOT))
        {
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            groundCollision = true;
        }

        if (aligned && pb.getTopEdge() <= csc_.m_collider.m_lowestSlopePoint && (overlap & utils::OverlapResult::OOT_SLOPE) && (overlap & utils::OverlapResult::OVERLAP_Y)) // Touched slope from right direction
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleSlope(obstacleId_))
                return;

            //std::cout << "Touched slope, teleporting on top, offset.x < 0\n";

            trans_.m_pos.y = highest;
            if (csc_.m_collider.m_topAngleCoef != 0)
                touchedSlope = csc_.m_collider.m_topAngleCoef;
            pb = phys_.m_pushbox + trans_.m_pos;
            groundCollision = true;

            if (phys_.m_velocity.y > 0)
                phys_.m_velocity.y = 0;
            if (phys_.m_inertia.y > 0)
                phys_.m_inertia.y = 0;
        }
        else if ((!aligned || csc_.m_collider.m_hasBox) && (overlap & utils::OverlapResult::OOT_BOX) && (overlap & utils::OverlapResult::OVERLAP_Y) && pb.getBottomEdge() > csc_.m_collider.m_lowestSlopePoint) // Touched inner box
        {
            if (obstacleId_ && !obsFallthrough_.touchedObstacleSide(obstacleId_))
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
                if (phys_.m_velocity.x < 0)
                    phys_.m_velocity.x = 0;
                if (phys_.m_inertia.x < 0)
                    phys_.m_inertia.x = 0;
            }
        }
    };

    // Iteration over colliders depending on archetype
    auto distrbObstacle = [&clds_, &reg = this->m_reg](auto idx, ComponentStaticCollider& cld_, auto &distrib_)
    {
        if (reg.all_of<ComponentObstacle>(idx))
            distrib_(cld_, reg.get<ComponentObstacle>(idx).m_obstacleId);
        else
            distrib_(cld_, 0);
    };

    // X axis movement handling
    {
        trans_.m_pos.x += offset.x;
        pb = phys_.m_pushbox + trans_.m_pos;

        // Moving to the right

        if (offset.x > 0)
        {
            for (auto [idx, cld] : clds_.each())
                distrbObstacle(idx, cld, resolveRight);
        }
        // Moving to the left
        else if (offset.x < 0)
        {
            for (auto [idx, cld] : clds_.each())
                distrbObstacle(idx, cld, resolveLeft);
        }
    }

    // Y axis movement handling
    {
        trans_.m_pos.y += offset.y;
        pb = phys_.m_pushbox + trans_.m_pos;

        // Falling / staying in place
        if (offset.y >= 0)
        {
            for (auto [idx, cld] : clds_.each())
                distrbObstacle(idx, cld, resolveFall);
        }
        // Rising
        else
        {
            if (noUpwardLanding)
            {
                touchedSlope = 0;
                groundCollision = false;
            }

            for (auto [idx, cld] : clds_.each())
                distrbObstacle(idx, cld, resolveRise);
        }
    }

    resetEntityObstacles(trans_, phys_, obsFallthrough_);
    phys_.m_lastSlopeAngle = phys_.m_onSlopeWithAngle;
    phys_.m_onSlopeWithAngle = touchedSlope;

    if (sm_)
    {
        auto *currentState = sm_->getRealCurrentState();

        if (groundCollision)
        {
            currentState->onTouchedGround({&m_reg, idx_});
        }
        else
        {
            if (!magnetEntity(clds_, trans_, phys_, obsFallthrough_))
                currentState->onLostGround({&m_reg, idx_});
        }
    }

    phys_.m_appliedOffset = trans_.m_pos - oldPos;

    obsFallthrough_.m_isIgnoringObstacles.update();
}

bool PhysicsSystem::magnetEntity(auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_)
{
    if (phys_.m_magnetLimit <= 0.0f)
        return false;

    auto pb = phys_.m_pushbox + trans_.m_pos;

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
            phys_.m_onSlopeWithAngle = pcld->m_topAngleCoef;
            return true;
        }
    }

    return false;
}

std::pair<bool, const SlopeCollider*> PhysicsSystem::getHighestVerticalMagnetCoord(auto &clds_, const Collider &cld_, float &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_)
{
    float baseCoord = coord_;
    float bot = cld_.getBottomEdge();
    bool isFound = false;
    const SlopeCollider *foundcld = nullptr;
    auto proceedCollider = [&foundcld, &ignoreAllObstacles_, &ignoredObstacles_, &baseCoord, &isFound, &coord_, &cld_](const SlopeCollider &areaCld_, int obstacleId_)
    {
        if (obstacleId_ && (ignoreAllObstacles_ || ignoredObstacles_.contains(obstacleId_)))
            return;

        auto horOverlap = utils::getOverlap<0>(areaCld_.m_points[0].x, areaCld_.m_points[1].x, cld_.getLeftEdge(), cld_.getRightEdge()); //cld.getHorizontalOverlap(cld_);
        if (!!(horOverlap & utils::OverlapResult::OVERLAP_X))
        {
            auto height = areaCld_.getTopHeight(cld_, horOverlap);
            if (height > baseCoord && (!isFound || height < coord_))
            {
                coord_ = height;
                isFound = true;
                foundcld = &areaCld_;
            }
        }
    };

    // Fall iteration over colliders depending on archetype
    auto distrbObstacle = [&proceedCollider, &reg = this->m_reg](auto idx, ComponentStaticCollider& cld_)
    {
        if (reg.all_of<ComponentObstacle>(idx))
        {
            proceedCollider(cld_.m_collider, reg.get<ComponentObstacle>(idx).m_obstacleId);
        }
        else
        {
            proceedCollider(cld_.m_collider, 0);
        }
    };

    clds_.each(distrbObstacle);

    return {isFound, foundcld};
}

void PhysicsSystem::resetEntityObstacles(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_)
{
    auto touched = getTouchedObstacles(phys_.m_pushbox + trans_.m_pos);
    std::set<int> res;
    std::set_intersection(
        obsFallthrough_.m_ignoredObstacles.begin(), obsFallthrough_.m_ignoredObstacles.end(),
        touched.begin(), touched.end(),
        std::inserter(res, res.begin()));

    //if (obsFallthrough_.m_ignoredObstacles.size() != res.size())
    //    std::cout << "Reseted obstacles\n";

    obsFallthrough_.m_ignoredObstacles = res;
}

std::set<int> PhysicsSystem::getTouchedObstacles(const Collider &pb_)
{
    auto viewObstacles = m_reg.view<ComponentStaticCollider, ComponentObstacle>();
    std::set<int> obstacleIds;
    float dumped = 0.0f;

    auto proceedObstacle = [&](const SlopeCollider &areaCld_, int obstacleId_) 
    {
        if (obstacleIds.contains(obstacleId_))
            return false;

        return !!(areaCld_.getFullCollisionWith(pb_, dumped) & utils::OverlapResult::BOTH_OOT);
    };

    for (auto [idx, cld, obst] : viewObstacles.each())
    {
        if (proceedObstacle(cld.m_collider, obst.m_obstacleId))
            obstacleIds.insert(obst.m_obstacleId);
    }

    return obstacleIds;
}
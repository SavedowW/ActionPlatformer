#include "DynamicColliderSystem.h"
#include "Core/CoreComponents.h"
#include "Core/Profile.h"

DynamicColliderSystem::DynamicColliderSystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void DynamicColliderSystem::updateSwitchingColliders()
{
    PROFILE_FUNCTION;

    auto clds = m_reg.view<ComponentStaticCollider, SwitchCollider>();

    for (auto [idx, cld, swc] : clds.each())
    {
        auto res = swc.updateTimer();
        if (res != cld.m_isEnabled)
        {
            bool overlapping = (cld.m_obstacleId ? isObstacleOverlappingWithDynamic(cld.m_resolved, cld.m_obstacleId) : isOverlappingWithDynamic(cld.m_resolved));
            if (!overlapping)
                cld.m_isEnabled = res;
        }
    }
}

void DynamicColliderSystem::updateMovingColliders()
{
    PROFILE_FUNCTION;
    
    auto routes = m_reg.view<MoveCollider2Points, ColliderRoutingIterator>();
    for (auto [idx, m2p, routing] : routes.each())
    {
        solveRouteIter(m2p, routing);
    }

    auto dynamics = m_reg.view<ComponentTransform, ComponentStaticCollider, MoveCollider2Points>();
    for (auto [idx, trans, scld, mvmnt] : dynamics.each())
    {
        proceedMovingCollider(trans, scld, mvmnt);
    }
}

void DynamicColliderSystem::proceedMovingCollider(ComponentTransform &trans_, ComponentStaticCollider &scld_, MoveCollider2Points &twop_)
{
    if (twop_.m_timer.isOver())
        return;

    twop_.m_timer.update();

    const Vector2<float> newtl = twop_.m_point2 + (twop_.m_point1 - twop_.m_point2) * twop_.m_timer.getProgressNormalized();

    moveColliderAt(trans_, scld_, newtl);
}

bool DynamicColliderSystem::isOverlappingWithDynamic(const SlopeCollider &cld_)
{
    const auto dynamics = m_reg.view<ComponentTransform, ComponentPhysical>();
    for (const auto [idx, trans, phys] : dynamics.each())
    {
        auto pb = phys.m_pushbox + trans.m_pos;
        int dump = 0;
        auto res = cld_.checkOverlap(pb, dump);
        if ((res & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH)
            return true;
    }

    return false;
}

bool DynamicColliderSystem::isObstacleOverlappingWithDynamic(const SlopeCollider &cld_, int obstacleId_)
{
    const auto dynamics = m_reg.view<ComponentTransform, ComponentPhysical>();
    for (const auto [idx, trans, phys] : dynamics.each())
    {
        auto pb = phys.m_pushbox + trans.m_pos;
        int dump = 0;
        auto res = cld_.checkOverlap(pb, dump);
        if ((res & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH)
        {
            if (!m_reg.all_of<ComponentObstacleFallthrough>(idx))
                return true;
            else
                m_reg.get<ComponentObstacleFallthrough>(idx).setIgnoreObstacle(obstacleId_);
        }
    }

    return false;
}

void DynamicColliderSystem::solveRouteIter(MoveCollider2Points &m2p_, ColliderRoutingIterator &iter_)
{
    bool firstTime = !m2p_.m_timer.isActive() && !m2p_.m_timer.isOver();

    if (firstTime || iter_.m_iter == iter_.m_route.m_links.size() - 1 && m2p_.m_timer.isOver())
    {
        iter_.m_iter = 0;

        m2p_.m_point2 = iter_.m_route.m_origin.m_pos - m2p_.m_offset;
        m2p_.m_point1 = iter_.m_route.m_links[0].m_target.m_pos - m2p_.m_offset;
        m2p_.m_timer.begin(iter_.m_route.m_links[0].m_duration);

        //std::cout << "Starting movement at " << iter_.m_iter << ": (" << m2p_.m_point1 << ") - (" << m2p_.m_point2 << ")" << std::endl;
    }
    else if (m2p_.m_timer.isOver())
    {
        iter_.m_iter++;

        m2p_.m_point2 = iter_.m_route.m_links[iter_.m_iter - 1].m_target.m_pos - m2p_.m_offset;
        m2p_.m_point1 = iter_.m_route.m_links[iter_.m_iter].m_target.m_pos - m2p_.m_offset;
        m2p_.m_timer.begin(iter_.m_route.m_links[iter_.m_iter].m_duration);

        //std::cout << "Continuing movement at " << iter_.m_iter << ": (" << m2p_.m_point1 << ") - (" << m2p_.m_point2 << ")" << std::endl;
    }
}

void DynamicColliderSystem::moveColliderAt(ComponentTransform &trans_, ComponentStaticCollider &scld_, const Vector2<int> &newtl_)
{
    const Vector2<float> offset = newtl_ - trans_.m_pos;

    // Resolved collider with full offset applied
    const SlopeCollider newcld = scld_.m_proto.movedBy(newtl_);

    // Resolved collider with only Y offset applied
    const SlopeCollider newcldYOnly = scld_.m_proto.movedBy({trans_.m_pos.x, newtl_.y});

    const auto dynamics = m_reg.view<ComponentTransform, ComponentPhysical>();
    for (auto [idx, trans, phys] : dynamics.each())
    {
        ComponentObstacleFallthrough *fallthrough = nullptr;

        if (scld_.m_obstacleId)
        {
            if (m_reg.all_of<ComponentObstacleFallthrough>(idx))
                fallthrough = &m_reg.get<ComponentObstacleFallthrough>(idx);
            else
                continue;
        }

        const Vector2<int> oldpos = trans.m_pos;
        auto pb = phys.m_pushbox + trans.m_pos;
        auto oldTop = pb.getTopEdge();
        auto oldRightEdge = pb.getRightEdge();
        auto oldLeftEdge = pb.getLeftEdge();
        int oldHighest = 0;
        int newHighest = 0;

        const bool attachedLeft = (phys.m_onWall != entt::null) && trans.m_orientation == ORIENTATION::LEFT && oldRightEdge + 1 == scld_.m_resolved.leftX()
                        && pb.m_topLeft.y + pb.m_size.y / 2 >= scld_.m_resolved.leftY() && pb.m_topLeft.y + pb.m_size.y / 2 <= scld_.m_resolved.bottomY();
        const bool attachedRight = (phys.m_onWall != entt::null) && trans.m_orientation == ORIENTATION::RIGHT && oldLeftEdge - 1 == scld_.m_resolved.rightX()
                        && pb.m_topLeft.y + pb.m_size.y / 2 >= scld_.m_resolved.rightY() && pb.m_topLeft.y + pb.m_size.y / 2 <= scld_.m_resolved.bottomY();
        

        auto oldColres = scld_.m_resolved.checkOverlap(pb, oldHighest);

        // If platform moved vertically
        if (offset.y != 0)
        {
            auto newColres = newcldYOnly.checkOverlap(pb, newHighest);
            bool collision = ((oldColres & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH || (newColres & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH);

            // If moving up
            if (offset.y < 0)
            {
                // If was above
                if (collision && oldpos.y < oldHighest)
                {
                    if (fallthrough && !fallthrough->touchedObstacleTop(scld_.m_obstacleId))
                        continue;
                    //std::cout << "Teleporting entity on top while moving platform up" << std::endl;
                    // Teleport on top
                    trans.m_pos.y = newHighest - 1;
                    pb = phys.m_pushbox + trans.m_pos;
                    phys.m_onMovingPlatform = true;
                }
            }
            // If moving down
            else if (offset.y > 0)
            {
                // If was standing on top
                if ((oldColres & OverlapResult::OVERLAP_X) == OverlapResult::OVERLAP_X && oldpos.y == oldHighest - 1)
                {
                    if (fallthrough && scld_.m_obstacleId && !fallthrough->touchedObstacleTop(scld_.m_obstacleId))
                        continue;

                    //std::cout << "Teleporting entity on top while moving platform down" << std::endl;
                    // Teleport on top
                    trans.m_pos.y = newHighest - 1;
                    pb = phys.m_pushbox + trans.m_pos;
                    phys.m_onMovingPlatform = true;
                }

                // If was below and now overlaps
                else if (collision && oldTop > scld_.m_resolved.bottomY())
                {
                    if (fallthrough && scld_.m_obstacleId && !fallthrough->touchedObstacleBottom(scld_.m_obstacleId))
                        continue;

                    //std::cout << "Teleporting entity to bottom while moving platform down" << std::endl;
                    // Teleport to bottom
                    trans.m_pos.y = newcld.bottomY() + pb.m_size.y;
                    pb = phys.m_pushbox + trans.m_pos;
                }
            }

            // Pull if the character is clinging to it
            if (attachedLeft || attachedRight)
            {
                //std::cout << "Attached, pushing in the movement direction (vertical)" << std::endl;
                if (trans.m_orientation == ORIENTATION::LEFT && oldRightEdge + 1 == scld_.m_resolved.leftX()
                        && pb.m_topLeft.y + pb.m_size.y / 2 >= scld_.m_resolved.leftY() && pb.m_topLeft.y + pb.m_size.y / 2 <= scld_.m_resolved.bottomY() ||
                    trans.m_orientation == ORIENTATION::RIGHT && oldLeftEdge - 1 == scld_.m_resolved.rightX()
                        && pb.m_topLeft.y + pb.m_size.y / 2 >= scld_.m_resolved.rightY() && pb.m_topLeft.y + pb.m_size.y / 2 <= scld_.m_resolved.bottomY())
                {
                    phys.m_extraoffset.y = (abs(phys.m_extraoffset.y) > abs(offset.y) ? phys.m_extraoffset.y : offset.y);
                }
            }
        }

        // If platform moved horizontally
        if (offset.x != 0)
        {
            auto newColres = newcld.checkOverlap(pb, newHighest);

            bool teleported = false;

            // If it moved to the right
            if (offset.x > 0)
            {
                // If now collides
                if ((newColres & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH)
                {
                    auto rightest = newcld.getMostRightAt(pb);
                    auto onSlope = !(rightest == newcldYOnly.rightX());

                    if (fallthrough &&  onSlope && scld_.m_obstacleId && !fallthrough->touchedObstacleSlope(scld_.m_obstacleId) ||
                        fallthrough && !onSlope && scld_.m_obstacleId && !fallthrough->touchedObstacleSide(scld_.m_obstacleId))
                        continue;

                    //std::cout << "Teleporting to right " << rand() << std::endl;
                    // Teleport to right edge
                    trans.m_pos.x = rightest + pb.m_size.x / 2;
                    pb = phys.m_pushbox + trans.m_pos;
                    teleported = true;
                }

                // If attached from left side, pull
                if (attachedLeft)
                {
                    //std::cout << "Attached, pushing in the movement direction" << std::endl;
                    phys.m_extraoffset.x = (abs(phys.m_extraoffset.x) > abs(offset.x) ? phys.m_extraoffset.x : offset.x);
                    teleported = true;
                }
            }

            // If it moved to the left
            else if (offset.x < 0)
            {
                auto leftest = newcld.getMostLeftAt(pb);
                auto onSlope = !(leftest == newcldYOnly.leftX());

                if (fallthrough &&  onSlope && scld_.m_obstacleId && !fallthrough->touchedObstacleSlope(scld_.m_obstacleId) ||
                    fallthrough && !onSlope && scld_.m_obstacleId && !fallthrough->touchedObstacleSide(scld_.m_obstacleId))
                    continue;

                // If now collides
                if ((newColres & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH)
                {
                    //std::cout << "Teleporting to left\n";
                    // Teleport to right edge
                    trans.m_pos.x = leftest - pb.m_size.x / 2 - 1;
                    pb = phys.m_pushbox + trans.m_pos;
                    teleported = true;
                }

                // If attached from right side, pull
                if (attachedRight)
                {
                    //std::cout << "Attached, pushing in the movement direction" << std::endl;
                    phys.m_extraoffset.x = (abs(phys.m_extraoffset.x) > abs(offset.x) ? phys.m_extraoffset.x : offset.x);
                    teleported = true;
                }
            }

            // If stands on it
            if (!teleported && (phys.m_onGround != entt::null) && (oldColres & OverlapResult::OVERLAP_X) == OverlapResult::OVERLAP_X && abs(trans.m_pos.y - newHighest) <= 1.0f)
            {
                if (fallthrough && scld_.m_obstacleId && !fallthrough->touchedObstacleTop(scld_.m_obstacleId))
                    continue;

                //std::cout << "Standing on top, pushing in the movement direction" << std::endl;

                phys.m_extraoffset.x = (abs(phys.m_extraoffset.x) > abs(offset.x) ? phys.m_extraoffset.x : offset.x);
                phys.m_onMovingPlatform = true;
            }
        }

        phys.m_enforcedOffset = (trans.m_pos - oldpos) + phys.m_extraoffset;
        phys.m_pushedOffset = (trans.m_pos - oldpos).mulComponents(1, 10);
        //std::cout << phys.m_extraoffset << std::endl;
    }

    scld_.m_resolved = newcld;
    trans_.m_pos = newtl_;
}

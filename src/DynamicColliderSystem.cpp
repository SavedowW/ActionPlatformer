#include "DynamicColliderSystem.h"
#include "CoreComponents.h"

DynamicColliderSystem::DynamicColliderSystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void DynamicColliderSystem::updateSwitchingColliders()
{
    auto clds = m_reg.view<ComponentStaticCollider, SwitchCollider>();

    for (auto [idx, cld, swc] : clds.each())
    {
        auto res = swc.updateTimer();
        if (res != cld.m_isEnabled)
        {
            bool overlapping = (cld.m_obstacleId ? isObstacleOverlappingWithDynamic(cld.m_collider, cld.m_obstacleId) : isOverlappingWithDynamic(cld.m_collider));
            if (!overlapping)
                cld.m_isEnabled = res;
        }
    }
}

void DynamicColliderSystem::updateMovingColliders()
{
    auto dynamics = m_reg.view<ComponentStaticCollider, MoveCollider2Points>();

    for (auto [idx, scld, mvmnt] : dynamics.each())
    {
        proceedMovingCollider(scld, mvmnt);
    }
}

void DynamicColliderSystem::proceedMovingCollider(ComponentStaticCollider &scld_, MoveCollider2Points &twop_)
{
    if (twop_.m_timer.update())
    {
        twop_.m_timer.begin(twop_.m_duration);
        twop_.m_toSecond = !twop_.m_toSecond;
    }

    Vector2<float> newtl;
    if (twop_.m_toSecond)
        newtl = twop_.m_point1 + (twop_.m_point2 - twop_.m_point1) * twop_.m_timer.getProgressNormalized() - scld_.m_collider.m_size / 2.0f;
    else
        newtl = twop_.m_point2 + (twop_.m_point1 - twop_.m_point2) * twop_.m_timer.getProgressNormalized() - scld_.m_collider.m_size / 2.0f;

    SlopeCollider newcld(newtl, scld_.m_collider.m_size, scld_.m_collider.m_topAngleCoef);

    Vector2<float> offset = newtl - scld_.m_collider.m_tlPos;

    const auto dynamics = m_reg.view<ComponentTransform, ComponentPhysical>();
    for (auto [idx, trans, phys] : dynamics.each())
    {
        Vector2<float> oldpos = trans.m_pos;
        auto pb = phys.m_pushbox + trans.m_pos;
        auto oldTop = pb.getTopEdge();
        auto oldRightEdge = pb.getRightEdge();
        auto oldLeftEdge = pb.getLeftEdge();
        float oldHighest = 0.0f;
        float newHighest = 0.0f;

        bool attachedLeft = phys.m_isAttached && trans.m_orientation == ORIENTATION::LEFT && abs(oldRightEdge - scld_.m_collider.m_points[0].x) <= 1.0f
                        && pb.m_center.y >= scld_.m_collider.m_points[0].y && pb.m_center.y <= scld_.m_collider.m_points[3].y;
        bool attachedRight = phys.m_isAttached && trans.m_orientation == ORIENTATION::RIGHT && abs(oldLeftEdge - scld_.m_collider.m_points[1].x) <= 1.0f
                        && pb.m_center.y >= scld_.m_collider.m_points[1].y && pb.m_center.y <= scld_.m_collider.m_points[2].y;
        

        auto oldColres = scld_.m_collider.getFullCollisionWith(pb, oldHighest);

        // If platform moved vertically
        if (offset.y != 0)
        {
            auto newColres = newcld.getFullCollisionWith(pb, newHighest);
            bool collision = (oldColres || newColres);

            // If moving up
            if (offset.y < 0)
            {
                // If was above and now below
                if (collision && oldpos.y <= oldHighest && oldpos.y > newHighest)
                {
                    std::cout << "Teleporting on top\n";
                    // Teleport on top
                    trans.m_pos.y = newHighest;
                    pb = phys.m_pushbox + trans.m_pos;
                    phys.m_onMovingPlatform = true;
                }
            }
            // If moving down
            else if (offset.y > 0)
            {
                auto heightdiff = abs(oldpos.y - oldHighest);

                // If close enough to top to be considered standing
                if (collision && heightdiff <= 0.01f)
                {
                    std::cout << "Teleporting on top\n";
                    // Teleport on top
                    newHighest = newcld.getTopHeight(pb, newColres);
                    trans.m_pos.y = newHighest;
                    pb = phys.m_pushbox + trans.m_pos;
                }

                // If was below and now overlaps
                else if (collision && scld_.m_obstacleId && oldTop >= scld_.m_collider.m_points[2].y && oldTop < newcld.m_points[2].y)
                {
                    std::cout << "Teleporting to bottom\n";
                    // Teleport to bottom
                    trans.m_pos.y = newcld.m_points[2].y + 2 * pb.m_halfSize.y;
                    pb = phys.m_pushbox + trans.m_pos;
                }
            }

            // Pull if the character is clinging to it
            if (attachedLeft || attachedRight)
            {
                if (trans.m_orientation == ORIENTATION::LEFT && abs(oldRightEdge - scld_.m_collider.m_points[0].x) <= 1.0f
                        && pb.m_center.y >= scld_.m_collider.m_points[0].y && pb.m_center.y <= scld_.m_collider.m_points[3].y ||
                    trans.m_orientation == ORIENTATION::RIGHT && abs(oldLeftEdge - scld_.m_collider.m_points[1].x) <= 1.0f
                        && pb.m_center.y >= scld_.m_collider.m_points[1].y && pb.m_center.y <= scld_.m_collider.m_points[2].y)
                {
                    phys.m_extraoffset.y += offset.y;
                }
            }
        }

        // If platform moved horizontally
        if (offset.x != 0)
        {
            auto newColres = newcld.getFullCollisionWith(pb, newHighest);
            bool collision = (oldColres & utils::OverlapResult::BOTH_OVERLAP) || (newColres & utils::OverlapResult::BOTH_OVERLAP);

            // If it moved to the right
            if (offset.x > 0)
            {
                // FIXME: when moving at a diagonal, this section is affected by the result of vertical offset
                
                // If didnt collide and now does
                if (!((oldColres & utils::OverlapResult::OVERLAP_X) && (oldColres & utils::OverlapResult::OVERLAP_Y)) && ((newColres & utils::OverlapResult::OVERLAP_X) && (newColres & utils::OverlapResult::OVERLAP_Y)))
                {
                    std::cout << "Teleporting to right\n";
                    // Teleport to right edge
                    trans.m_pos.x = newcld.getMostRightAt(pb) + pb.m_halfSize.x;
                    pb = phys.m_pushbox + trans.m_pos;
                }

                // If attached from left side, pull
                if (attachedLeft)
                    phys.m_extraoffset.x += offset.x;
            }

            // If it moved to the left
            else if (offset.x < 0)
            {
                // If didnt collide and now does
                if (!((oldColres & utils::OverlapResult::OVERLAP_X) && (oldColres & utils::OverlapResult::OVERLAP_Y)) && ((newColres & utils::OverlapResult::OVERLAP_X) && (newColres & utils::OverlapResult::OVERLAP_Y)))
                {
                    std::cout << "Teleporting to left\n";
                    // Teleport to right edge
                    trans.m_pos.x = newcld.getMostLeftAt(pb) - pb.m_halfSize.x;
                    pb = phys.m_pushbox + trans.m_pos;
                }

                // If attached from right side, pull
                if (attachedRight)
                    phys.m_extraoffset.x += offset.x;
            }

            // If stands on it
            if (phys.m_isGrounded && (oldColres & utils::OverlapResult::OVERLAP_X) && abs(oldpos.y - oldHighest) <= 1.0f)
            {
                trans.m_pos.x += offset.x;
                phys.m_onMovingPlatform = true;
            }
        }

        phys.m_enforcedOffset = (trans.m_pos - oldpos) + phys.m_extraoffset;
    }

    scld_.m_collider = newcld;
}

bool DynamicColliderSystem::isOverlappingWithDynamic(const SlopeCollider &cld_)
{
    const auto dynamics = m_reg.view<ComponentTransform, ComponentPhysical>();
    for (const auto [idx, trans, phys] : dynamics.each())
    {
        auto pb = phys.m_pushbox + trans.m_pos;
        float dump = 0.0f;
        auto res = cld_.getFullCollisionWith(pb, dump);
        if ((res & utils::OverlapResult::OVERLAP_X) && (res & utils::OverlapResult::OVERLAP_Y))
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
        float dump = 0.0f;
        auto res = cld_.getFullCollisionWith(pb, dump);
        if ((res & utils::OverlapResult::OVERLAP_X) && (res & utils::OverlapResult::OVERLAP_Y))
        {
            if (!m_reg.all_of<ComponentObstacleFallthrough>(idx))
                return true;
            else
                m_reg.get<ComponentObstacleFallthrough>(idx).setIgnoreObstacle(obstacleId_);
        }
    }

    return false;
}

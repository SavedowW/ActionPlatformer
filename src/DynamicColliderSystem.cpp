#include "DynamicColliderSystem.h"
#include "CoreComponents.h"
#include "Profile.h"

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
    
    auto dynamics = m_reg.view<ComponentTransform, ComponentStaticCollider, MoveCollider2Points>();

    for (auto [idx, trans, scld, mvmnt] : dynamics.each())
    {
        proceedMovingCollider(trans, scld, mvmnt);
    }
}

// TODO: sometimes character moves to the edge of the platform while standing on top of it

void DynamicColliderSystem::proceedMovingCollider(ComponentTransform &trans_, ComponentStaticCollider &scld_, MoveCollider2Points &twop_)
{
    if (twop_.m_timer.update())
    {
        twop_.m_timer.begin(twop_.m_duration);
        twop_.m_toSecond = !twop_.m_toSecond;
    }

    Vector2<float> newtl;
    if (twop_.m_toSecond)
        newtl = twop_.m_point1 + (twop_.m_point2 - twop_.m_point1) * twop_.m_timer.getProgressNormalized() - scld_.m_proto.m_size / 2.0f;
    else
        newtl = twop_.m_point2 + (twop_.m_point1 - twop_.m_point2) * twop_.m_timer.getProgressNormalized() - scld_.m_proto.m_size / 2.0f;

    SlopeCollider newcld(newtl, scld_.m_proto.m_size, scld_.m_proto.m_topAngleCoef);
    SlopeCollider newcldYOnly(Vector2{trans_.m_pos.x, newtl.y}, scld_.m_proto.m_size, scld_.m_proto.m_topAngleCoef);

    Vector2<float> offset = newtl - trans_.m_pos;

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

        Vector2<float> oldpos = trans.m_pos;
        auto pb = phys.m_pushbox + trans.m_pos;
        auto oldTop = pb.getTopEdge();
        auto oldRightEdge = pb.getRightEdge();
        auto oldLeftEdge = pb.getLeftEdge();
        float oldHighest = 0.0f;
        float newHighest = 0.0f;

        bool attachedLeft = (phys.m_onWall != entt::null) && trans.m_orientation == ORIENTATION::LEFT && abs(oldRightEdge - scld_.m_resolved.m_points[0].x) <= 1.0f
                        && pb.m_center.y >= scld_.m_resolved.m_points[0].y && pb.m_center.y <= scld_.m_resolved.m_points[3].y;
        bool attachedRight = (phys.m_onWall != entt::null) && trans.m_orientation == ORIENTATION::RIGHT && abs(oldLeftEdge - scld_.m_resolved.m_points[1].x) <= 1.0f
                        && pb.m_center.y >= scld_.m_resolved.m_points[1].y && pb.m_center.y <= scld_.m_resolved.m_points[2].y;
        

        auto oldColres = scld_.m_resolved.checkOverlap(pb, oldHighest);

        // If platform moved vertically
        if (offset.y != 0)
        {
            auto newColres = newcldYOnly.checkOverlap(pb, newHighest);
            bool collision = (checkCollision(oldColres, CollisionResult::OVERLAP_BOTH) || checkCollision(newColres, CollisionResult::OVERLAP_BOTH));

            // If moving up
            if (offset.y < 0)
            {
                // If was close enough to top to be considered standing
                if (collision && oldpos.y - oldHighest <= 0.01f)
                {
                    if (fallthrough && !fallthrough->touchedObstacleTop(scld_.m_obstacleId))
                        continue;
                    std::cout << "Teleporting on top while moving up" << std::endl;
                    // Teleport on top
                    trans.m_pos.y = newHighest;
                    pb = phys.m_pushbox + trans.m_pos;
                    phys.m_onMovingPlatform = true;
                }
            }
            // If moving down
            else if (offset.y > 0)
            {
                // If was close enough to top to be considered standing
                if (checkCollision(oldColres, CollisionResult::OVERLAP_X) && abs(oldpos.y - oldHighest) <= 0.01f)
                {
                    if (fallthrough && scld_.m_obstacleId && !fallthrough->touchedObstacleTop(scld_.m_obstacleId))
                        continue;

                    std::cout << "Teleporting on top while moving down" << std::endl;
                    // Teleport on top
                    trans.m_pos.y = newHighest;
                    pb = phys.m_pushbox + trans.m_pos;
                }

                // If was below and now overlaps
                else if (collision && oldTop >= scld_.m_resolved.m_points[2].y && oldTop < newcld.m_points[2].y)
                {
                    if (fallthrough && scld_.m_obstacleId && !fallthrough->touchedObstacleBottom(scld_.m_obstacleId))
                        continue;

                    std::cout << "Teleporting to bottom while moving down" << std::endl;
                    // Teleport to bottom
                    trans.m_pos.y = newcld.m_points[2].y + 2 * pb.m_halfSize.y;
                    pb = phys.m_pushbox + trans.m_pos;
                }
            }

            // Pull if the character is clinging to it
            if (attachedLeft || attachedRight)
            {
                std::cout << "Attached, pushing in the movement direction (vertical)" << std::endl;
                if (trans.m_orientation == ORIENTATION::LEFT && abs(oldRightEdge - scld_.m_resolved.m_points[0].x) <= 1.0f
                        && pb.m_center.y >= scld_.m_resolved.m_points[0].y && pb.m_center.y <= scld_.m_resolved.m_points[3].y ||
                    trans.m_orientation == ORIENTATION::RIGHT && abs(oldLeftEdge - scld_.m_resolved.m_points[1].x) <= 1.0f
                        && pb.m_center.y >= scld_.m_resolved.m_points[1].y && pb.m_center.y <= scld_.m_resolved.m_points[2].y)
                {
                    phys.m_extraoffset.y = (abs(phys.m_extraoffset.y) > abs(offset.y) ? phys.m_extraoffset.y : offset.y);
                }
            }
        }

        // If platform moved horizontally
        if (offset.x != 0)
        {
            float oldHighestYOffset = 0.0f;
            auto oldColresYOnly = newcldYOnly.checkOverlap(pb, oldHighestYOffset);
            auto newColres = newcld.checkOverlap(pb, newHighest);
            bool collision = (checkCollision(oldColres, CollisionResult::OVERLAP_BOTH) || checkCollision(newColres, CollisionResult::OVERLAP_BOTH));

            bool teleported = false;

            // If it moved to the right
            if (offset.x > 0)
            {
                // If now collides
                if (checkCollision(newColres, CollisionResult::OVERLAP_BOTH))
                {
                    auto rightest = newcld.getMostRightAt(pb);
                    auto onSlope = !(rightest == newcldYOnly.m_points[1].x);

                    if (fallthrough && onSlope && scld_.m_obstacleId && !fallthrough->touchedObstacleSlope(scld_.m_obstacleId))
                        continue;

                    else if (fallthrough && !onSlope && scld_.m_obstacleId && !fallthrough->touchedObstacleSide(scld_.m_obstacleId))
                        continue;

                    std::cout << "Teleporting to right " << rand() << std::endl; // TODO: collides while standing on top
                    // Teleport to right edge
                    trans.m_pos.x = rightest + pb.m_halfSize.x;
                    pb = phys.m_pushbox + trans.m_pos;
                    teleported = true;
                }

                // If attached from left side, pull
                if (attachedLeft)
                {
                    std::cout << "Attached, pushing in the movement direction" << std::endl;
                    phys.m_extraoffset.x = (abs(phys.m_extraoffset.x) > abs(offset.x) ? phys.m_extraoffset.x : offset.x);
                    teleported = true;
                }
            }

            // If it moved to the left
            else if (offset.x < 0)
            {
                auto leftest = newcld.getMostLeftAt(pb);
                auto onSlope = !(leftest == newcldYOnly.m_points[0].x);

                if (fallthrough && onSlope && scld_.m_obstacleId && !fallthrough->touchedObstacleSlope(scld_.m_obstacleId))
                    continue;

                else if (fallthrough && !onSlope && scld_.m_obstacleId && !fallthrough->touchedObstacleSide(scld_.m_obstacleId))
                    continue;

                // If now collides
                if (checkCollision(newColres, CollisionResult::OVERLAP_BOTH))
                {
                    std::cout << "Teleporting to left\n";
                    // Teleport to right edge
                    trans.m_pos.x = leftest - pb.m_halfSize.x;
                    pb = phys.m_pushbox + trans.m_pos;
                    teleported = true;
                }

                // If attached from right side, pull
                if (attachedRight)
                {
                    std::cout << "Attached, pushing in the movement direction" << std::endl;
                    phys.m_extraoffset.x = (abs(phys.m_extraoffset.x) > abs(offset.x) ? phys.m_extraoffset.x : offset.x);
                    teleported = true;
                }
            }

            // If stands on it
            if (!teleported && (phys.m_onGround != entt::null) && checkCollision(oldColres, CollisionResult::OVERLAP_X) && abs(trans.m_pos.y - newHighest) <= 1.0f)
            {
                if (fallthrough && scld_.m_obstacleId && !fallthrough->touchedObstacleTop(scld_.m_obstacleId))
                    continue;

                std::cout << "Standing on top, pushing in the movement direction" << std::endl;

                phys.m_extraoffset.x = (abs(phys.m_extraoffset.x) > abs(offset.x) ? phys.m_extraoffset.x : offset.x);
                phys.m_onMovingPlatform = true;
            }
        }

        phys.m_enforcedOffset = (trans.m_pos - oldpos) + phys.m_extraoffset;
        std::cout << phys.m_extraoffset << std::endl;
    }

    scld_.m_resolved = newcld;
    trans_.m_pos = newtl;
}

bool DynamicColliderSystem::isOverlappingWithDynamic(const SlopeCollider &cld_)
{
    const auto dynamics = m_reg.view<ComponentTransform, ComponentPhysical>();
    for (const auto [idx, trans, phys] : dynamics.each())
    {
        auto pb = phys.m_pushbox + trans.m_pos;
        float dump = 0.0f;
        auto res = cld_.checkOverlap(pb, dump);
        if (checkCollision(res, CollisionResult::OVERLAP_BOTH))
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
        auto res = cld_.checkOverlap(pb, dump);
        if (checkCollision(res, CollisionResult::OVERLAP_BOTH))
        {
            if (!m_reg.all_of<ComponentObstacleFallthrough>(idx))
                return true;
            else
                m_reg.get<ComponentObstacleFallthrough>(idx).setIgnoreObstacle(obstacleId_);
        }
    }

    return false;
}

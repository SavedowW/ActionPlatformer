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
        auto oldpb = phys.m_pushbox + trans.m_pos;
        auto oldTop = oldpb.getTopEdge();
        auto oldRightEdge = oldpb.getRightEdge();
        auto oldLeftEdge = oldpb.getLeftEdge();
        float oldHighest = 0.0f;
        float newHighest = 0.0f;

        auto oldColres = scld_.m_collider.getFullCollisionWith(oldpb, oldHighest);
        auto newColres = newcld.getFullCollisionWith(oldpb, newHighest);

        // If nothing happened
        if (!oldColres && !newColres)
            continue;

        // If moving up
        if (offset.y < 0)
        {
            // If was above and now below
            if (oldpos.y <= oldHighest && oldpos.y > newHighest)
            {
                std::cout << "Teleporting on top\n";
                // Teleport on top
                trans.m_pos.y = newHighest;
            }
        }
        // If moving down
        else if (offset.y > 0)
        {
            auto heightdiff = abs(oldpos.y - oldHighest);

            // If close enough to top to be considered standing
            if (heightdiff <= 0.01f)
            {
                std::cout << "Teleporting on top\n";
                // Teleport on top
                newHighest = newcld.getTopHeight(oldpb, newColres);
                trans.m_pos.y = newHighest;
            }

            // If was below and now overlaps
            if (scld_.m_obstacleId && oldTop >= scld_.m_collider.m_points[2].y && oldTop < newcld.m_points[2].y)
            {
                std::cout << "Teleporting to bottom\n";
                // Teleport to bottom
                trans.m_pos.y = newcld.m_points[2].y + 2 * oldpb.m_halfSize.y;
            }
        }

        phys.m_enforcedOffset += (trans.m_pos - oldpos);
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

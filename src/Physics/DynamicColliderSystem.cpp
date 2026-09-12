#include "DynamicColliderSystem.h"
#include "Core/Collider.h"
#include "Core/CoreComponents.h"


DynamicCollidersHandler::DynamicCollidersHandler(const DCS::MovingCollidersView &movingCollidersView_, const DCS::DynamicsView &dynamicsView_, const CollidersView &collidersView_) :
    m_movingCollidersView{movingCollidersView_},
    m_dynamicsView{dynamicsView_},
    m_collidersView{collidersView_}
{}

void DynamicCollidersHandler::Handle()
{
    for (const auto &[idx, trans, scld, mvmnt] : m_movingCollidersView.each())
    {
        if (mvmnt.timer.isOver())
            continue;

        mvmnt.timer.update();

        const Vector2<int> delta = (mvmnt.point1 - mvmnt.point2).abs() * mvmnt.timer.getProgressNormalized();

        Vector2<int> newtl = mvmnt.point2;
        if (mvmnt.point1.x >= mvmnt.point2.x)
            newtl.x += delta.x;
        else
            newtl.x -= delta.x;

        if (mvmnt.point1.y >= mvmnt.point2.y)
            newtl.y += delta.y;
        else
            newtl.y -= delta.y;

        moveColliderAt(idx, trans, scld, newtl);
    }

    for (auto &el : m_handlers)
    {
        if (el.second.physics().peekRawOffset().y >= 0.0f)
            el.second.magnet();
        el.second.discoverPosition(false);
    }
}

void DynamicCollidersHandler::moveColliderAt(entt::entity cid_, ComponentTransform &trans_, ComponentStaticCollider &scld_, const Vector2<int> &newtl_)
{
    const Vector2<int> offset = newtl_ - trans_.m_pos;

    const auto horizontallyMoved = scld_.m_resolved.movedBy({offset.x, 0});
    const auto fullyMoved = scld_.m_resolved.movedBy(offset);

    for (const auto &[idx, trans, phys, worldPos, fallthrough] : m_dynamicsView.each())
    {
        if (fallthrough.isIgnoringObstacle(idx) || fallthrough.isIgnoringAllObstacles() && scld_.obstacleType > ObstacleType::MINIMAL)
            continue;

        const auto originalPos = trans.m_pos;
        auto newPb = phys.pushbox + trans.m_pos;
        
        int highest = trans.m_pos.y;
        const auto initialOverlap = scld_.m_resolved.checkOverlap(newPb, highest);

        if (scld_.obstacleType < ObstacleType::FLOOR) // If it even can push horizontally and is clingable
        {
            if (offset.x > 0) // To the right
            {
                const Vector2<int> rightPoint{newPb.getRightEdge() + 1, newPb.getTopEdge() + newPb.m_size.y / 2};

                if (worldPos.wall.clingState == ClingState::RIGHT &&
                    scld_.m_resolved.leftX() == rightPoint.x && 
                    scld_.m_resolved.leftY() <= rightPoint.y && 
                    scld_.m_resolved.bottomY() >= rightPoint.y || // If clinging to the left wall at initial position:
                    (initialOverlap & OverlapResult::OVERLAP_X) == OverlapResult::OVERLAP_X &&
                    highest - 1 == trans.m_pos.y && (worldPos.ground.onGround != entt::null))  // if used to stand on top:
                {
                    // Soft move alongside it
                    getDynamicHandler(idx, trans, phys, fallthrough, worldPos).moveRight(offset.x, false);
                    newPb = phys.pushbox + trans.m_pos;
                }
                else if ((horizontallyMoved.checkOverlap(newPb, highest) & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH) // Else if overlaps with X moved collider:
                {
                    //   Hard move to the right boundary
                    getDynamicHandler(idx, trans, phys, fallthrough, worldPos).moveRight(horizontallyMoved.rightX() + 1 - newPb.getLeftEdge(), true);
                    newPb = phys.pushbox + trans.m_pos;
                }
            }
            else if (offset.x < 0) // To the left
            {
                const Vector2<int> leftPoint{newPb.getLeftEdge() - 1, newPb.getTopEdge() + newPb.m_size.y / 2};

                if (worldPos.wall.clingState == ClingState::LEFT &&
                    scld_.m_resolved.rightX() == leftPoint.x &&
                    scld_.m_resolved.rightY() <= leftPoint.y &&
                    scld_.m_resolved.bottomY() >= leftPoint.y || // If clinging to the right wall at initial position:
                    (initialOverlap & OverlapResult::OVERLAP_X) == OverlapResult::OVERLAP_X &&
                    highest - 1 == trans.m_pos.y && (worldPos.ground.onGround != entt::null))  // if used to stand on top:
                {
                    // Soft move alongside it
                    getDynamicHandler(idx, trans, phys, fallthrough, worldPos).moveLeft(-offset.x, false);
                    newPb = phys.pushbox + trans.m_pos;
                }
                else if ((horizontallyMoved.checkOverlap(newPb) & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH) // Else if overlaps with X moved collider:
                {
                    // Hard move to the left boundary
                    getDynamicHandler(idx, trans, phys, fallthrough, worldPos).moveLeft(newPb.getRightEdge() + 1 - horizontallyMoved.leftX(), true);
                    newPb = phys.pushbox + trans.m_pos;
                }
            }
        }

        // TODO: consider clinging and standing on top of the collider
        if (offset.y > 0 && scld_.obstacleType >= ObstacleType::MINIMAL) // Downward
        {
            if ((fullyMoved.checkOverlap(newPb) & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH)
            {
                getDynamicHandler(idx, trans, phys, fallthrough, worldPos).moveDown(fullyMoved.bottomY() + 1 - newPb.getTopEdge(), true);
                newPb = phys.pushbox + trans.m_pos;
            }
        }
        else if (offset.y < 0) // Upward
        {
            int upHighest = 0;
            if ((fullyMoved.checkOverlap(newPb, upHighest) & OverlapResult::OVERLAP_BOTH) == OverlapResult::OVERLAP_BOTH)
            {
                getDynamicHandler(idx, trans, phys, fallthrough, worldPos).moveUp(newPb.getBottomEdge() + 1 - upHighest);
                newPb = phys.pushbox + trans.m_pos;
            }
        }

        phys.pendingEnforcedOffset += trans.m_pos - originalPos;
    }

    scld_.m_resolved = fullyMoved;
    trans_.m_pos = newtl_;
}

PhysicsEntityHandler &DynamicCollidersHandler::getDynamicHandler(entt::entity idx_, ComponentTransform &trans_, 
        ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, WorldPosition &worldPos_)
{
    const auto found = m_handlers.find(idx_);
    if (found != m_handlers.end())
        return found->second;

    return m_handlers.emplace(std::piecewise_construct, std::forward_as_tuple(idx_), std::forward_as_tuple(m_collidersView, trans_, phys_, obsFallthrough_, worldPos_)).first->second;
}



DynamicColliderSystem::DynamicColliderSystem(entt::registry &reg_) :
    m_reg(reg_)
{
}

void DynamicColliderSystem::updateMovingColliders()
{
    auto routes = m_reg.view<MoveCollider2Points, ColliderRoutingIterator>();
    for (auto [idx, m2p, routing] : routes.each())
        solveRouteIter(m2p, routing);

    DynamicCollidersHandler{m_reg.view<ComponentTransform, ComponentStaticCollider, MoveCollider2Points>(),
                            m_reg.view<ComponentTransform, ComponentPhysical, WorldPosition, ComponentObstacleFallthrough>(),
                            m_reg.view<ComponentStaticCollider>()}.Handle();
}

void DynamicColliderSystem::solveRouteIter(MoveCollider2Points &m2p_, ColliderRoutingIterator &iter_)
{
    bool firstTime = !m2p_.timer.isActive() && !m2p_.timer.isOver();

    if (firstTime || iter_.m_iter == iter_.m_route.m_links.size() - 1 && m2p_.timer.isOver())
    {
        iter_.m_iter = 0;

        m2p_.point2 = iter_.m_route.m_origin.m_pos - m2p_.offset;
        m2p_.point1 = iter_.m_route.m_links[0].m_target.m_pos - m2p_.offset;
        m2p_.timer.begin(iter_.m_route.m_links[0].m_duration);

        //std::cout << "Starting movement at " << iter_.m_iter << ": (" << m2p_.point1 << ") - (" << m2p_.point2 << ")" << std::endl;
    }
    else if (m2p_.timer.isOver())
    {
        iter_.m_iter++;

        m2p_.point2 = iter_.m_route.m_links[iter_.m_iter - 1].m_target.m_pos - m2p_.offset;
        m2p_.point1 = iter_.m_route.m_links[iter_.m_iter].m_target.m_pos - m2p_.offset;
        m2p_.timer.begin(iter_.m_route.m_links[iter_.m_iter].m_duration);

        //std::cout << "Continuing movement at " << iter_.m_iter << ": (" << m2p_.point1 << ") - (" << m2p_.point2 << ")" << std::endl;
    }
}

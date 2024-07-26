#include "BattleLevel.h"
#include "Application.h"

BattleLevel::BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_) :
    Level(application_, size_, lvlId_),
    m_camera({0.0f, 0.0f}, gamedata::global::baseResolution, m_size),
    m_decor(application_),
    m_tlmap(application_)
{
    m_hud.addWidget(std::make_unique<DebugDataWidget>(*m_application, m_camera, lvlId_, size_, m_lastFrameTimeMS));

    m_tlmap.load("Tiles/Tilemap-sheet");

    DecorationBuilder bld(*application_);
    m_decor = std::move(bld.buildDecorLayers("Resources/Sprites/Tiles/map.json", m_tlmap));
}

void BattleLevel::enter()
{
    Level::enter();

    m_camera.setScale(gamedata::global::minCameraScale);
    m_camera.setPos({0.0f, 0.0f});

    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{0, 20}, Vector2{6, 1}, 0));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{6, 20}, Vector2{1, 1}, -1, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{7, 19}, Vector2{1, 1}, -1, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{8, 18}, Vector2{1, 1}, -1, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{9, 17}, Vector2{1, 1}, -1, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{10, 16}, Vector2{1, 1}, -1, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{11, 15}, Vector2{2, 1}, 0));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{13, 15}, Vector2{1, 1}, -0.5));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{14.0f, 14.5f}, Vector2{9, 1}, 0));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{23.0f, 14.5f}, Vector2{1, 1}, 0.5));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{24, 15}, Vector2{1, 1}, 0));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{25, 15}, Vector2{6, 6}, 1, 2));
    //m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{26, 16}, Vector2{1, 1}, 1, 2));
    //m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{27, 17}, Vector2{1, 1}, 1, 2));
    //m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{28, 18}, Vector2{1, 1}, 1, 2));
    //m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{29, 19}, Vector2{1, 1}, 1, 2));
    //m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{30, 20}, Vector2{1, 1}, 1, 2));

    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{6, 20}, Vector2{1, 1}, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{7, 21}, Vector2{1, 1}, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{8, 22}, Vector2{1, 1}, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{9, 23}, Vector2{1, 1}, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{10, 24}, Vector2{1, 1}, 1));

    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{11, 25}, Vector2{15, 1}, 0));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{26, 25}, Vector2{1, 1}, -1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{27, 24}, Vector2{1, 1}, -1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{28, 23}, Vector2{1, 1}, 0));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{29, 23}, Vector2{1, 1}, -1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{30, 22}, Vector2{1, 1}, -1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{31, 21}, Vector2{9, 9}, 1));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{40, 30}, Vector2{5, 1}, 0));

    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{45, 6}, Vector2{1, 25}, 0));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{40, 1}, Vector2{1, 23}, 0));

    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{41, 6}, Vector2{5, 1}, 0, 3));

    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{41, 0}, Vector2{10, 1}, 0));

    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{50, 30}, Vector2{6, 1}, 0));
    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{69, 30}, Vector2{6, 1}, 0));

    m_camFocusAreas.emplace_back(getTilePos(Vector2{43, 16}), gamedata::global::tileSize.mulComponents(Vector2{40.0f, 32.0f}), *m_application->getRenderer());
    m_camFocusAreas.back().overrideFocusArea({getTilePos(Vector2{42.5f, 15.0f}), gamedata::global::tileSize.mulComponents(Vector2{7.0f, 30.0f}) / 2.0f});

    m_camFocusAreas.emplace_back(getTilePos(Vector2{58, 16}), gamedata::global::tileSize.mulComponents(Vector2{40.0f, 32.0f}), *m_application->getRenderer());
    m_camFocusAreas.back().overrideFocusArea({getTilePos(Vector2{58.0f, 15.0f}), gamedata::global::tileSize.mulComponents(Vector2{24.0f, 30.0f}) / 2.0f});

    m_camFocusAreas.emplace_back(getTilePos(Vector2{76, 16}), gamedata::global::tileSize.mulComponents(Vector2{40.0f, 32.0f}), *m_application->getRenderer());
    m_camFocusAreas.back().overrideFocusArea({getTilePos(Vector2{80.25f, 15.0f}), gamedata::global::tileSize.mulComponents(Vector2{20.5f, 30.0f}) / 2.0f});

    m_collisionArea.finalize();
    
}

void BattleLevel::update()
{
    /*for (auto &chr : m_actionCharacters)
    {
        chr->update();

        auto &transform = chr->getComponent<ComponentTransform>();
        auto &physical = chr->getComponent<ComponentPhysical>();
        auto &obshandle = chr->getComponent<ComponentObstacleFallthrough>();

        const auto offset = physical.getPosOffest();
        Collider pb = physical.getPushbox();
        bool noUpwardLanding = chr->getNoUpwardLanding();

        auto oldHeight = transform.m_pos.y;
        auto oldTop = pb.getTopEdge();
        auto oldRightEdge = pb.getRightEdge();
        auto oldLeftEdge = pb.getLeftEdge();

        bool groundCollision = false;
        float touchedSlope = 0.0f;
        float highest = m_size.y;

        {
            transform.m_pos.x += offset.x;
            pb = physical.getPushbox();
            if (offset.x > 0)
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    auto overlap = cld.getFullCollisionWith(physical.getPushbox(), highest);
                    bool aligned = cld.getOrientationDir() > 0;
                    auto heightDiff = abs(cld.m_lowestSlopePoint - pb.getBottomEdge());

                    if ((overlap & utils::OverlapResult::TOUCH_MIN1_MAX2) && (overlap & utils::OverlapResult::BOTH_OOT))
                    {
                        if (cld.m_topAngleCoef != 0)
                            touchedSlope = cld.m_topAngleCoef;
                        groundCollision = true;
                    }

                    if (aligned && pb.getTopEdge() <= cld.m_lowestSlopePoint && (overlap & utils::OverlapResult::OOT_SLOPE) && (overlap & utils::OverlapResult::OVERLAP_Y)) // Touched slope from right direction
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleSlope(cld.m_obstacleId))
                            continue;

                        std::cout << "Touched slope, teleporting on top, offset.x > 0\n";

                        transform.m_pos.y = highest;
                        if (cld.m_topAngleCoef != 0)
                            touchedSlope = cld.m_topAngleCoef;
                        pb = physical.getPushbox();
                        groundCollision = true;

                        if (physical.m_velocity.y > 0)
                            physical.m_velocity.y = 0;
                        if (physical.m_inertia.y > 0)
                            physical.m_inertia.y = 0;
                    }
                    else if ((!aligned || cld.m_hasBox) && (overlap & utils::OverlapResult::OOT_BOX) && (overlap & utils::OverlapResult::OVERLAP_Y) && pb.getBottomEdge() > cld.m_lowestSlopePoint) // Touched inner box
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleSide(cld.m_obstacleId))
                            continue;

                        std::cout << "Touched edge, teleporting to it, offset.x > 0\n";

                        transform.m_pos.x = cld.m_tlPos.x - pb.m_halfSize.x;
                        pb = physical.getPushbox();
                        if (physical.m_velocity.x < 0)
                            physical.m_velocity.x = 0;
                        if (physical.m_inertia.x < 0)
                            physical.m_inertia.x = 0;
                    }
                }
            }
            else if (offset.x < 0)
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    auto overlap = cld.getFullCollisionWith(physical.getPushbox(), highest);
                    bool aligned = cld.getOrientationDir() < 0;
                    auto heightDiff = abs(cld.m_lowestSlopePoint - pb.getBottomEdge());

                    if ((overlap & utils::OverlapResult::TOUCH_MIN1_MAX2) && (overlap & utils::OverlapResult::BOTH_OOT))
                    {
                        if (cld.m_topAngleCoef != 0)
                            touchedSlope = cld.m_topAngleCoef;
                        groundCollision = true;
                    }

                    if (aligned && pb.getTopEdge() <= cld.m_lowestSlopePoint && (overlap & utils::OverlapResult::OOT_SLOPE) && (overlap & utils::OverlapResult::OVERLAP_Y)) // Touched slope from right direction
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleSlope(cld.m_obstacleId))
                            continue;

                        std::cout << "Touched slope, teleporting on top, offset.x < 0\n";

                        transform.m_pos.y = highest;
                        if (cld.m_topAngleCoef != 0)
                            touchedSlope = cld.m_topAngleCoef;
                        pb = physical.getPushbox();
                        groundCollision = true;

                        if (physical.m_velocity.y > 0)
                            physical.m_velocity.y = 0;
                        if (physical.m_inertia.y > 0)
                            physical.m_inertia.y = 0;
                    }
                    else if ((!aligned || cld.m_hasBox) && (overlap & utils::OverlapResult::OOT_BOX) && (overlap & utils::OverlapResult::OVERLAP_Y) && pb.getBottomEdge() > cld.m_lowestSlopePoint) // Touched inner box
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleSide(cld.m_obstacleId))
                            continue;

                        std::cout << "Touched edge, teleporting to it, offset.x < 0\n";

                        transform.m_pos.x = cld.m_tlPos.x + cld.m_size.x + pb.m_halfSize.x;
                        pb = physical.getPushbox();
                        if (physical.m_velocity.x < 0)
                            physical.m_velocity.x = 0;
                        if (physical.m_inertia.x < 0)
                            physical.m_inertia.x = 0;
                    }
                }
            }
        }

        {
            transform.m_pos.y += offset.y;
            pb = physical.getPushbox();
            if (offset.y < 0) // TODO: fully reset upward velocity and inertia if hitting the ceiling far from the corner
            {
                if (noUpwardLanding)
                {
                    touchedSlope = 0;
                    groundCollision = false;
                }

                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    if (cld.m_highestSlopePoint > oldTop)
                        continue;

                    auto overlap = cld.getFullCollisionWith(pb, highest);
                    if ((overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y))
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleBottom(cld.m_obstacleId))
                            continue;

                        std::cout << "Touched ceiling, teleporting to bottom, offset.y < 0\n";

                        transform.m_pos.y = cld.m_points[2].y + pb.getSize().y;
                        pb = physical.getPushbox();
                    }
                }
            }
            else
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    auto overlap = cld.getFullCollisionWith(physical.getPushbox(), highest);
                    if ((overlap & utils::OverlapResult::OVERLAP_X) && (overlap & utils::OverlapResult::OOT_Y))
                    {
                        if (cld.m_obstacleId && (!obshandle.touchedObstacleTop(cld.m_obstacleId) || highest < oldHeight))
                            continue;

                        std::cout << "Touched slope top, teleporting on top, offset.y > 0\n";

                        transform.m_pos.y = highest;
                        if (cld.m_topAngleCoef != 0)
                            touchedSlope = cld.m_topAngleCoef;
                        groundCollision = true;
                        pb = physical.getPushbox();

                        if (physical.m_velocity.y > 0)
                            physical.m_velocity.y = 0;
                        if (physical.m_inertia.y > 0)
                            physical.m_inertia.y = 0;
                    }
                }
            }
        }

        obshandle.cleanIgnoredObstacles();
        physical.m_onSlopeWithAngle = touchedSlope;

        if (groundCollision)
        {
            chr->onTouchedGround();
        }
        else
        {
            if (!physical.attemptResetGround())
                chr->onLostGround();
        }
    }*/

    if (updateFocus())
    {
        //m_camera.smoothMoveTowards(m_currentCamFocusArea->getCameraTargetPosition(m_pc->getCameraFocusPoint()), {1.0f, 1.0f}, 0, 1.3f, 20.0f);
        m_camera.smoothScaleTowards(m_currentCamFocusArea->getScale());
    }
    else
    {
        //m_camera.smoothMoveTowards(m_pc->getCameraFocusPoint(), {1.0f, 0.5f}, 5.0f, 1.6f, 80.0f);
        m_camera.smoothScaleTowards(gamedata::global::maxCameraScale);
    }
    m_camera.update();

    m_hud.update();
}

void BattleLevel::draw()
{
    auto &renderer = *m_application->getRenderer();
    renderer.prepareRenderer(SDL_Color{ 31, 24, 51, 255 });

    m_decor.draw(m_camera);

    if (gamedata::debug::drawColliders)
    {
        for (const auto &cld : m_collisionArea.m_staticCollisionMap)
        {
            if (cld.m_obstacleId)
                renderer.drawCollider(cld, {50, 50, 255, 100}, 255, m_camera);
            else
                renderer.drawCollider(cld, {255, 0, 0, 100}, 255, m_camera);
        }

        for (const auto &trg : m_collisionArea.m_triggers)
        {
            renderer.drawCollider(trg, {255, 50, 255, 50}, 100, m_camera);
        }
    }

    //for (auto &el : m_actionCharacters)
    //    el->getComponent<ComponentAnimationRenderable>().draw(m_camera);

    if (gamedata::debug::drawFocusAreas)
    {
        for (auto &cfa : m_camFocusAreas)
            cfa.draw(m_camera);
    }

    renderer.switchToHUD({0, 0, 0, 0});
    m_hud.draw(renderer, m_camera);

    renderer.updateScreen(m_camera);
}

bool BattleLevel::updateFocus()
{
    /*auto pb = m_pc->getComponent<ComponentPhysical>().getPushbox();
    if (m_currentCamFocusArea)
    {
        if (m_currentCamFocusArea->checkIfEnters(pb, true))
            return true;
        else
            m_currentCamFocusArea = nullptr;
    }

    for (auto &cfa: m_camFocusAreas)
    {
        if (cfa.checkIfEnters(pb, false))
        {
            m_currentCamFocusArea = &cfa;
            return true;
        }
    }*/

    return false;
}

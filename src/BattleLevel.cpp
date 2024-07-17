#include "BattleLevel.h"
#include "Application.h"
#include "Enemy1.h"

BattleLevel::BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_) :
    Level(application_, size_, lvlId_),
    m_camera({0.0f, 0.0f}, gamedata::global::baseResolution, m_size),
    m_decor(application_),
    m_tlmap(application_)
{
    m_pc = new PlayableCharacter(*application_, m_collisionArea);
    m_actionCharacters.push_back(std::unique_ptr<MovableCharacter>(m_pc));

    m_actionCharacters.push_back(std::unique_ptr<MovableCharacter>(new Enemy1(*application_, m_collisionArea)));
    m_actionCharacters[1]->setOnLevel(*m_application, getTileCenter(Vector2{31, 16}));

    m_hud.addWidget(std::make_unique<DebugDataWidget>(*m_application, m_camera, lvlId_, size_, m_lastFrameTimeMS));
    m_hud.addWidget(std::make_unique<DebugPlayerWidget>(*m_application, m_camera, m_pc));

    m_tlmap.load("Tiles/Tilemap-sheet");

    DecorationBuilder bld(*application_);
    m_decor = std::move(bld.buildDecorLayers("Resources/Sprites/Tiles/map.json", m_tlmap));
}

void BattleLevel::enter()
{
    Level::enter();

    m_pc->setOnLevel(*m_application, getTileCenter(Vector2{29, 18}));
    m_camera.setScale(gamedata::global::minCameraScale);
    m_camera.setPos(m_pc->getComponent<ComponentTransform>().m_pos);

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

    m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{50, 30}, Vector2{10, 1}, 0));

    m_camFocusAreas.emplace_back(Vector2{700.0f, 250.0f}, Vector2{550.0f, 500.0f}, *m_application->getRenderer());
    m_camFocusAreas.back().overrideFocusArea({m_camFocusAreas.back().x + 150, m_camFocusAreas.back().y, m_camFocusAreas.back().w - 400, m_camFocusAreas.back().h});

    m_camFocusAreas.emplace_back(Vector2{825.0f, 250.0f}, Vector2{550.0f, 500.0f}, *m_application->getRenderer());
    m_camFocusAreas.back().overrideFocusArea({m_camFocusAreas.back().x + 175, m_camFocusAreas.back().y, m_camFocusAreas.back().w - 300, m_camFocusAreas.back().h});

    m_collisionArea.finalize();
    
}

void BattleLevel::update()
{
    for (auto &chr : m_actionCharacters)
    {
        chr->update();

        auto &transform = chr->getComponent<ComponentTransform>();
        auto &physical = chr->getComponent<ComponentPhysical>();
        auto &obshandle = chr->getComponent<ComponentObstacleFallthrough>();

        const auto offset = physical.getPosOffest();
        Collider pb = physical.getPushbox();

        auto oldHeight = transform.m_pos.y;
        auto oldTop = pb.y;
        auto oldRightEdge = pb.x + pb.w;
        auto oldLeftEdge = pb.x;

        bool groundCollision = false;
        float touchedSlope = 0.0f;
        float highest = m_size.y;

        {
            transform.m_pos.y += offset.y;
            pb = physical.getPushbox();
            if (offset.y < 0)
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    if (cld.getFullCollisionWith<true, false>(physical.getPushbox(), highest))
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleBottom(cld.m_obstacleId) || cld.m_highestSlopePoint > oldTop)
                            continue;

                        transform.m_pos.y = cld.m_points[2].y + pb.h;
                        pb = physical.getPushbox();
                    }
                }
            }
            else
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    if (cld.getFullCollisionWith<true, false>(physical.getPushbox(), highest))
                    {
                        if (cld.m_obstacleId && (!obshandle.touchedObstacleTop(cld.m_obstacleId) || highest < oldHeight))
                            continue;

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

        {
            transform.m_pos.x += offset.x;
            pb = physical.getPushbox();
            if (offset.x > 0)
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    auto colres = cld.getFullCollisionWith<false, true, true>(physical.getPushbox(), highest);
                    bool aligned = cld.getOrientationDir() > 0;
                    if (colres == 1 && aligned) // Touched slope from right direction
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleSlope(cld.m_obstacleId))
                            continue;

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
                    else if (colres) // Touched inner box
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleSide(cld.m_obstacleId))
                            continue;

                        std::cout << "Touched edge, teleporting to it, offset.x > 0\n";

                        transform.m_pos.x = cld.m_tlPos.x - pb.w / 2.0f;
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
                    auto colres = cld.getFullCollisionWith<false, true, true>(physical.getPushbox(), highest);
                    bool aligned = cld.getOrientationDir() < 0;
                    if (colres == 1 && aligned) // Touched slope from right direction
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleSlope(cld.m_obstacleId))
                            continue;

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
                    else if (colres) // Touched inner box
                    {
                        if (cld.m_obstacleId && !obshandle.touchedObstacleSide(cld.m_obstacleId))
                            continue;

                        std::cout << "Touched edge, teleporting to it, offset.x < 0\n";

                        transform.m_pos.x = cld.m_tlPos.x + cld.m_size.x + pb.w / 2.0f;
                        pb = physical.getPushbox();
                        if (physical.m_velocity.x < 0)
                            physical.m_velocity.x = 0;
                        if (physical.m_inertia.x < 0)
                            physical.m_inertia.x = 0;
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
    }

    if (updateFocus())
    {
        m_camera.smoothMoveTowards(m_currentCamFocusArea->getCameraTargetPosition(m_pc->getCameraFocusPoint()), {1.0f, 1.0f}, 0, 1.3f, 20.0f);
        m_camera.smoothScaleTowards(m_currentCamFocusArea->getScale());
    }
    else
    {
        m_camera.smoothMoveTowards(m_pc->getCameraFocusPoint(), {1.0f, 0.5f}, 5.0f, 1.6f, 80.0f);
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

    for (auto &el : m_actionCharacters)
        el->getComponent<ComponentAnimationRenderable>().draw(m_camera);

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
    auto pb = m_pc->getComponent<ComponentPhysical>().getPushbox();
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
    }

    return false;
}

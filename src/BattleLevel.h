#ifndef BATTLE_LEVEL_H_
#define BATTLE_LEVEL_H_

#include "CameraFocusArea.h"
#include "Level.h"
#include "Camera.h"
#include "Background.h"
#include "Application.h"
#include <array>
#include "FrameTimer.h"
#include <SDL_ttf.h>
#include <ranges>
#include <set>
#include "DebugDataWidget.h"
#include "DebugPlayerWidget.h"
#include "PlayableCharacter.h"
#include "CollisionArea.h"
#include "DecorationBuilder.h"

template <typename BackType>
class BattleLevel : public Level
{
public:
    BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_) :
    	Level(application_, size_, lvlId_),
    	m_camera({0.0f, 0.0f}, gamedata::global::baseResolution, m_size),
        m_pc(*application_, getTileCenter(Vector2{30, 19}), m_collisionArea),
        m_decor(application_),
        m_tlmap(application_)
    {
        static_assert(std::is_base_of_v<Background, BackType>, "BackType of BattleLevel should be derived from Background class");

        m_hud.addWidget(std::make_unique<DebugDataWidget>(*m_application, m_camera, lvlId_, size_, m_lastFrameTimeMS));
        m_hud.addWidget(std::make_unique<DebugPlayerWidget>(*m_application, m_camera, &m_pc));

        m_tlmap.load("Tiles/Tilemap-sheet");

        DecorationBuilder bld(*application_);
        m_decor = std::move(bld.buildDecorLayers("Resources/Sprites/Tiles/map.json", m_tlmap));
    }

    void receiveInput(EVENTS event, const float scale_) override
    {
        Level::receiveInput(event, scale_);
    }

    virtual void enter() override
    {
        Level::enter();

        m_pc.setOnLevel(*m_application);
        m_camera.setScale(gamedata::global::minCameraScale);
        m_camera.setPos(m_pc.accessPos());

        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{0, 20}, Vector2{6, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{6, 20}, Vector2{1, 1}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{7, 19}, Vector2{1, 1}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{8, 18}, Vector2{1, 1}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{9, 17}, Vector2{1, 1}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{10, 16}, Vector2{1, 0}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{11, 15}, Vector2{2, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{13, 15}, Vector2{1, 1}, -0.5));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{14.0f, 14.5f}, Vector2{9, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{23.0f, 14.5f}, Vector2{1, 1}, 0.5));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{24, 15}, Vector2{1, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{25, 15}, Vector2{1, 1}, 1, 2));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{26, 16}, Vector2{1, 1}, 1, 2));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{27, 17}, Vector2{1, 1}, 1, 2));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{28, 18}, Vector2{1, 1}, 1, 2));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{29, 19}, Vector2{1, 1}, 1, 2));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{30, 20}, Vector2{1, 1}, 1, 2));

        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{6, 20}, Vector2{1, 1}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{7, 21}, Vector2{1, 1}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{8, 22}, Vector2{1, 1}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{9, 23}, Vector2{1, 1}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{10, 24}, Vector2{1, 1}, 1));

        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{11, 25}, Vector2{15, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{26, 25}, Vector2{1, 0}, -1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{27, 24}, Vector2{1, 0}, -1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{28, 23}, Vector2{1, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{29, 23}, Vector2{1, 0}, -1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{30, 22}, Vector2{1, 0}, -1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{31, 21}, Vector2{50, 1}, 0));

        m_camFocusAreas.emplace_back(getTileCenter(Vector2{19.0f, 10.0f}), gamedata::global::minCameraSize.y, *m_application->getRenderer());
        
    }

    virtual ~BattleLevel()
    {
    }

protected:
    void update() override
    {
        m_pc.update();

        const auto offset = m_pc.getPosOffest();
        auto &pos = m_pc.accessPos();
        Collider pb = m_pc.getPushbox();

        auto oldHeight = pos.y;
        auto oldTop = pb.y;
        auto oldRightEdge = pb.x + pb.w;
        auto oldLeftEdge = pb.x;

        m_pc.isIgnoringAllObstacles();

        bool groundCollision = false;
        float highest = m_size.y;

        auto &vel = m_pc.accessVelocity();
        auto &inr = m_pc.accessInertia();


        {
            pos.y += offset.y;
            pb = m_pc.getPushbox();
            if (offset.y < 0)
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    //if (m_pc.getPushbox().checkCollisionWith<true, false>(m_staticCollisionMap[*first]))
                    if (cld.getFullCollisionWith<true, false>(m_pc.getPushbox(), highest))
                    {
                        if (cld.m_obstacleId && !m_pc.touchedObstacleBottom(cld.m_obstacleId) || cld.m_highestSlopePoint > oldTop)
                            continue;

                        pos.y = cld.m_points[2].y+ pb.h;
                        pb = m_pc.getPushbox();
                    }
                }
            }
            else
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    //if (m_pc.getPushbox().checkCollisionWith<true, false>(m_staticCollisionMap[*first]))
                    if (cld.getFullCollisionWith<true, false>(m_pc.getPushbox(), highest))
                    {
                        if (cld.m_obstacleId && (!m_pc.touchedObstacleTop(cld.m_obstacleId) || highest < oldHeight))
                            continue;

                        pos.y = highest;
                        groundCollision = true;
                        pb = m_pc.getPushbox();

                        if (vel.y > 0)
                            vel.y = 0;
                        if (inr.y > 0)
                            inr.y = 0;
                    }
                }
            }
        }

        {
            pos.x += offset.x;
            pb = m_pc.getPushbox();
            if (offset.x > 0)
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    auto colres = cld.getFullCollisionWith<false, true, true>(m_pc.getPushbox(), highest);
                    bool aligned = cld.getOrientationDir() > 0;
                    if (colres == 1 && aligned) // Touched slope from right direction
                    {
                        if (cld.m_obstacleId && !m_pc.touchedObstacleSlope(cld.m_obstacleId))
                            continue;

                        pos.y = highest;
                        pb = m_pc.getPushbox();
                        groundCollision = true;

                        if (vel.y > 0)
                            vel.y = 0;
                        if (inr.y > 0)
                            inr.y = 0;
                    }
                    else if (colres) // Touched inner box
                    {
                        if (cld.m_obstacleId && !m_pc.touchedObstacleSide(cld.m_obstacleId))
                            continue;

                        std::cout << "Touched edge, teleporting to it, offset.x > 0\n";

                        pos.x = cld.m_tlPos.x - pb.w / 2.0f;
                        pb = m_pc.getPushbox();
                        if (vel.x < 0)
                            vel.x = 0;
                        if (inr.x < 0)
                            inr.x = 0;
                    }
                }
            }
            else if (offset.x < 0)
            {
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    auto colres = cld.getFullCollisionWith<false, true, true>(m_pc.getPushbox(), highest);
                    bool aligned = cld.getOrientationDir() < 0;
                    if (colres == 1 && aligned) // Touched slope from right direction
                    {
                        if (cld.m_obstacleId && !m_pc.touchedObstacleSlope(cld.m_obstacleId))
                            continue;

                        pos.y = highest;
                        pb = m_pc.getPushbox();
                        groundCollision = true;

                        if (vel.y > 0)
                            vel.y = 0;
                        if (inr.y > 0)
                            inr.y = 0;
                    }
                    else if (colres) // Touched inner box
                    {
                        if (cld.m_obstacleId && !m_pc.touchedObstacleSide(cld.m_obstacleId))
                            continue;

                        std::cout << "Touched edge, teleporting to it, offset.x < 0\n";

                        pos.x = cld.m_tlPos.x + cld.m_size.x + pb.w / 2.0f;
                        pb = m_pc.getPushbox();
                        if (vel.x < 0)
                            vel.x = 0;
                        if (inr.x < 0)
                            inr.x = 0;
                    }
                }
            }
        }

        m_pc.cleanIgnoredObstacles();

        if (groundCollision)
        {
            m_pc.onTouchedGround();
        }
        else
        {
            if (!m_pc.attemptResetGround())
                m_pc.onLostGround();
        }

        if (updateFocus())
        {
            m_camera.smoothMoveTowards(m_currentCamFocusArea->getCenter(), 0, 1.3f, 20.0f);
            m_camera.smoothScaleTowards(m_currentCamFocusArea->getScale());
        }
        else
        {
            m_camera.smoothMoveTowards(m_pc.getCameraFocusPoint(), 5.0f, 1.6f, 80.0f);
            m_camera.smoothScaleTowards(gamedata::global::maxCameraScale);
        }
        m_camera.update();

        m_hud.update();

        //std::cout << m_characters[0]->CharStateData() << " | " << m_characters[1]->CharStateData() << std::endl;
    }

    void draw() override
    {
    	auto &renderer = *m_application->getRenderer();
    	renderer.prepareRenderer(SDL_Color{ 31, 24, 51, 255 });

    	if (m_background.get())
    		m_background->draw(renderer, m_camera);

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
        }

        m_pc.draw(m_camera);

        if (gamedata::debug::drawFocusAreas)
        {
            for (auto &cfa : m_camFocusAreas)
                cfa.draw(m_camera);
        }

        renderer.switchToHUD({0, 0, 0, 0});
        m_hud.draw(renderer, m_camera);

    	renderer.updateScreen(m_camera);
    }

    bool updateFocus()
    {
        auto pb = m_pc.getPushbox();
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

    HUD m_hud;
    Camera m_camera;
    std::unique_ptr<BackType> m_background;

    PlayableCharacter m_pc;

    CollisionArea m_collisionArea;

    std::vector<CameraFocusArea> m_camFocusAreas;
    CameraFocusArea *m_currentCamFocusArea = nullptr;

    DecorLayers m_decor;
    Tileset m_tlmap;
};

#endif
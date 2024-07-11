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
#include "DecorLayers.hpp"

template<typename T, typename F, typename... Args>
void makeUniqueLinearRange(T& t_, F(T::* f_)(std::unique_ptr<StaticDecor>&&), Vector2<float> source_, const Vector2<float> &dir_, int iters_, Args&&... args_)
{
    while (iters_--)
    {
        (t_.*f_)(std::make_unique<StaticDecor>(std::forward<Args>(args_)..., getTileCenter(source_)));

        source_ += dir_;
    }
}

template <typename BackType>
class BattleLevel : public Level
{
public:
    BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_) :
    	Level(application_, size_, lvlId_),
    	m_camera({0.0f, 0.0f}, gamedata::global::baseResolution, m_size),
        m_pc(*application_, getTileCenter(Vector2{6, 59}), m_collisionArea),
        m_decor(application_)
    {
        static_assert(std::is_base_of_v<Background, BackType>, "BackType of BattleLevel should be derived from Background class");

        m_hud.addWidget(std::make_unique<DebugDataWidget>(*m_application, m_camera, lvlId_, size_, m_lastFrameTimeMS));
        m_hud.addWidget(std::make_unique<DebugPlayerWidget>(*m_application, m_camera, &m_pc));

        int texids[] = {
            application_->getTextureManager()->getTexID("Tiles/Tile5"), // light plain ground
            application_->getTextureManager()->getTexID("Tiles/Tile4"), // Upper ladder part
            application_->getTextureManager()->getTexID("Tiles/Tile6"), // Lower ladder part
            application_->getTextureManager()->getTexID("Tiles/Tile1"), // Transition to ladder up
            application_->getTextureManager()->getTexID("Tiles/Tile2"), // Transition to ladder down
            application_->getTextureManager()->getTexID("Tiles/Tile3"), // Small corner tile below ladder
        };

        makeUniqueLinearRange(m_decor, &DecorLayers<1>::insert<0>, Vector2{1.0f, 61.0f}, Vector2{1.0f, 0.0f}, 8, application_, texids[0], SDL_FLIP_NONE);

        //m_decor.insert<0>(std::make_unique<StaticDecor>(application_,  texids[3], SDL_FLIP_NONE, getTileCenter(Vector2{8, 61})));
        makeUniqueLinearRange(m_decor, &DecorLayers<1>::insert<0>, Vector2{8.0f, 60.0f}, Vector2{1.0f, -1.0f}, 5, application_, texids[1], SDL_FLIP_NONE);
        makeUniqueLinearRange(m_decor, &DecorLayers<1>::insert<0>, Vector2{9.0f, 60.0f}, Vector2{1.0f, -1.0f}, 4, application_, texids[2], SDL_FLIP_NONE);

        m_decor.insert<0>(std::make_unique<StaticDecor>(application_,  texids[4], SDL_FLIP_HORIZONTAL, getTileCenter(Vector2{9, 61})));
        makeUniqueLinearRange(m_decor, &DecorLayers<1>::insert<0>, Vector2{10.0f, 61.0f}, Vector2{1.0f, 1.0f}, 5, application_, texids[1], SDL_FLIP_HORIZONTAL);
        makeUniqueLinearRange(m_decor, &DecorLayers<1>::insert<0>, Vector2{10.0f, 62.0f}, Vector2{1.0f, 1.0f}, 4, application_, texids[2], SDL_FLIP_HORIZONTAL);

        m_decor.insert<0>(std::make_unique<StaticDecor>(application_,  texids[4], SDL_FLIP_NONE, getTileCenter(Vector2{13, 56})));
        makeUniqueLinearRange(m_decor, &DecorLayers<1>::insert<0>, Vector2{14.0f, 56.0f}, Vector2{1.0f, 0.0f}, 5, application_, texids[0], SDL_FLIP_NONE);
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

        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{0, 60}, Vector2{8, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{8, 60}, Vector2{1, 1}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{9, 59}, Vector2{1, 1}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{10, 58}, Vector2{1, 1}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{11, 57}, Vector2{1, 1}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{12, 56}, Vector2{1, 1}, -1, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{13, 55}, Vector2{4, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{17, 55}, Vector2{1, 1}, -0.5));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{18.0f, 54.5f}, Vector2{16, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{34.0f, 54.5f}, Vector2{1, 1}, 0.5));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{35, 55}, Vector2{3, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{38, 55}, Vector2{1, 1}, 1, 2));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{39, 56}, Vector2{1, 1}, 1, 2));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{40, 57}, Vector2{1, 1}, 1, 2));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{41, 58}, Vector2{1, 1}, 1, 2));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{42, 59}, Vector2{1, 1}, 1, 2));

        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{8, 60}, Vector2{1, 1}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{9, 61}, Vector2{1, 1}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{10, 62}, Vector2{1, 1}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{11, 63}, Vector2{1, 1}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{12, 64}, Vector2{1, 1}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{13, 65}, Vector2{2, 1}, 0.5));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{15, 66}, Vector2{16, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{31, 66}, Vector2{12, 1}, -0.5));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{43, 60}, Vector2{8, 1}, 0));

        m_camFocusAreas.emplace_back(getTileCenter(Vector2{26.5f, 50.0f}), gamedata::global::minCameraSize.y, *m_application->getRenderer());

        /*
          In current version, this sequence leads character into float because he only saves 0.5 slope which is not enough to magnet him closer
        addStaticCollider(getColliderForTileRange(Vector2{17.0f, 7.5f}, Vector2{1, 1}, 0.5));
        addStaticCollider(getColliderForTileRange(Vector2{18.0f, 8.0f}, Vector2{1, 1}, 1));
        addStaticCollider(getColliderForTileRange(Vector2{19.0f, 9.0f}, Vector2{3, 1}, 0));
        */
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
                    }
                    else if (colres) // Touched inner box
                    {
                        if (cld.m_obstacleId && !m_pc.touchedObstacleSide(cld.m_obstacleId) || cld.m_points[1].x < oldRightEdge)
                            continue;

                        std::cout << "Touched edge, teleporting to it, offset.x > 0\n";

                        pos.x = cld.m_tlPos.x - pb.w / 2.0f;
                        pb = m_pc.getPushbox();
                        vel.x = 0;
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
                    }
                    else if (colres) // Touched inner box
                    {
                        if (cld.m_obstacleId && !m_pc.touchedObstacleSide(cld.m_obstacleId) || cld.m_points[0].x > oldLeftEdge)
                            continue;

                        std::cout << "Touched edge, teleporting to it, offset.x < 0\n";

                        pos.x = cld.m_tlPos.x + cld.m_size.x + pb.w / 2.0f;
                        pb = m_pc.getPushbox();
                        vel.x = 0;
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
            m_camera.smoothMoveTowards(m_currentCamFocusArea->getCenter());
            m_camera.smoothScaleTowards(m_currentCamFocusArea->getScale());
        }
        else
        {
            m_camera.smoothMoveTowards(m_pc.getCameraFocusPoint());
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

        m_decor.draw<0>(m_camera);

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

    DecorLayers<1> m_decor;
};

#endif
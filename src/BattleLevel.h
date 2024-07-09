#ifndef BATTLE_LEVEL_H_
#define BATTLE_LEVEL_H_

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

template <typename BackType>
class BattleLevel : public Level
{
public:
    BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_) :
    	Level(application_, size_, lvlId_),
    	m_camera({0.0f, 0.0f}, gamedata::global::baseResolution, m_size),
        m_pc(*application_, getTileCenter(Vector2{10, 9}), m_collisionArea)
    {
        static_assert(std::is_base_of_v<Background, BackType>, "BackType of BattleLevel should be derived from Background class");

        m_hud.addWidget(std::make_unique<DebugDataWidget>(*m_application, m_camera, lvlId_, size_, m_lastFrameTimeMS));
        m_hud.addWidget(std::make_unique<DebugPlayerWidget>(*m_application, m_camera, &m_pc));
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

        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{2, 12}, Vector2{8, 2}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{0, 8}, Vector2{2, 2}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{10, 9}, Vector2{2, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{12, 9}, Vector2{1, 1}, -1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{13, 8}, Vector2{1, 1}, -0.5));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{14.0f, 7.5f}, Vector2{3, 1}, 0));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{17.0f, 7.5f}, Vector2{1.5f, 2.0f}, 1));
        m_collisionArea.addStaticCollider(getColliderForTileRange(Vector2{18.5f, 9.0f}, Vector2{5.0f, 1.0f}, 0));

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

        bool groundCollision = false;
        float highest = m_size.y;
        std::vector<std::reference_wrapper<SlopeCollider>> groundCollided;

        {
            pos.y += offset.y;
            for (auto &cld : m_collisionArea.m_staticCollisionMap)
            {
                //if (m_pc.getPushbox().checkCollisionWith<true, false>(m_staticCollisionMap[*first]))
                if (cld.getFullCollisionWith<true, false>(m_pc.getPushbox(), highest))
                {
                    pos.y = highest;
                    groundCollision = true;
                    groundCollided.push_back(cld);
                }
            }
        }

        {
            pos.x += offset.x;
            if (offset.x > 0)
            {
                auto pb = m_pc.getPushbox();
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    auto colres = cld.getFullCollisionWith<false, true>(m_pc.getPushbox(), highest);
                    bool aligned = cld.getOrientationDir() > 0;
                    if (colres == 1 && aligned) // Touched slope from right direction
                    {
                        pos.y = highest;
                        pb = m_pc.getPushbox();
                        groundCollision = true;
                        groundCollided.push_back(cld);
                    }
                    else if (colres)
                    {
                        pos.x = cld.m_tlPos.x - pb.w / 2.0f;
                        pb = m_pc.getPushbox();
                        //vel.x = 0;
                    }
                }
            }
            else if (offset.x < 0)
            {
                auto pb = m_pc.getPushbox();
                for (auto &cld : m_collisionArea.m_staticCollisionMap)
                {
                    auto colres = cld.getFullCollisionWith<false, true>(m_pc.getPushbox(), highest);
                    bool aligned = cld.getOrientationDir() < 0;
                    if (colres == 1 && aligned) // Touched slope from right direction
                    {
                        pos.y = highest;
                        pb = m_pc.getPushbox();
                        groundCollision = true;
                        groundCollided.push_back(cld);
                    }
                    else if (colres)
                    {
                        pos.x = cld.m_tlPos.x + cld.m_size.x + pb.w / 2.0f;
                        pb = m_pc.getPushbox();
                        //vel.x = 0;
                    }
                }
            }
        }

        if (groundCollision)
        {
            m_pc.onTouchedGround();
        }
        else
        {
            if (!m_pc.attemptResetGround())
                m_pc.onLostGround();
        }

        m_camera.smoothMoveTowards(m_pc.accessPos());
        m_camera.smoothScaleTowards(gamedata::global::maxCameraScale);
        m_camera.update();

        m_hud.update();

        //std::cout << m_characters[0]->CharStateData() << " | " << m_characters[1]->CharStateData() << std::endl;
    }

    void draw() override
    {
    	auto &renderer = *m_application->getRenderer();
    	renderer.prepareRenderer(SDL_Color{ 60, 197, 255, 255 });

    	if (m_background.get())
    		m_background->draw(renderer, m_camera);

        for (const auto &cld : m_collisionArea.m_staticCollisionMap)
        {
            renderer.drawCollider(cld, {255, 0, 0, 100}, 100, m_camera);
        }

        m_pc.draw(m_camera);


        m_hud.draw(renderer, m_camera);

    	renderer.updateScreen();
    }

    HUD m_hud;
    Camera m_camera;
    std::unique_ptr<BackType> m_background;

    PlayableCharacter m_pc;

    CollisionArea m_collisionArea;
};

#endif
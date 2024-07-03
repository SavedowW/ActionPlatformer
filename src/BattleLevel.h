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
#include "PlayableCharacter.h"

inline Collider getColliderForTileRange(Vector2<int> pos_, Vector2<int> size_)
{
    return {gamedata::global::tileSize.mulComponents(pos_), gamedata::global::tileSize.mulComponents(size_)};
}

inline Vector2<float> getTileCenter(Vector2<int> pos_)
{
    return gamedata::global::tileSize.mulComponents(pos_) - gamedata::global::tileSize / 2;
}

template <typename BackType>
class BattleLevel : public Level
{
public:
    BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_) :
    	Level(application_, size_, lvlId_),
    	m_camera({0.0f, 0.0f}, gamedata::global::baseResolution, m_size),
        m_pc(*application_, getTileCenter({4, 10})),
        m_staticCollisionMapTop([&m_staticCollisionMap = this->m_staticCollisionMap](int lhs_, int rhs_) {
            return m_staticCollisionMap[lhs_].y < m_staticCollisionMap[rhs_].y;
        }),
        m_staticCollisionMapLeft([&m_staticCollisionMap = this->m_staticCollisionMap](int lhs_, int rhs_) {
            return m_staticCollisionMap[lhs_].x < m_staticCollisionMap[rhs_].x;
        }),
        m_staticCollisionMapRight([&m_staticCollisionMap = this->m_staticCollisionMap](int lhs_, int rhs_) {
            return m_staticCollisionMap[lhs_].x + m_staticCollisionMap[lhs_].w < m_staticCollisionMap[rhs_].x + m_staticCollisionMap[rhs_].w;
        })
    {
        static_assert(std::is_base_of_v<Background, BackType>, "BackType of BattleLevel should be derived from Background class");

        m_hud.addWidget(std::make_unique<DebugDataWidget>(*m_application, m_camera, lvlId_, size_, m_lastFrameTimeMS));
    }

    void receiveInput(EVENTS event, const float scale_) override
    {
        Level::receiveInput(event, scale_);
    }

    virtual void enter() override
    {
        Level::enter();

        m_pc.setOnLevel(*m_application);
        m_camera.setPos({0, 0});
        m_camera.setScale(gamedata::global::maxCameraScale);

        addStaticCollider(getColliderForTileRange({2, 12}, {8, 2}));
        addStaticCollider(getColliderForTileRange({0, 8}, {2, 2}));
        addStaticCollider(getColliderForTileRange({10, 9}, {2, 1}));
    }

    virtual ~BattleLevel()
    {
    }

protected:
    void update() override
    {
        m_pc.update();

        auto &vel = m_pc.accessVelocity();
        auto &pos = m_pc.accessPos();

        {
            pos.y += vel.y;
            if (vel.y > 0)
            {
                auto first = std::find_if(m_staticCollisionMapTop.begin(), m_staticCollisionMapTop.end(), [&pos, &m_staticCollisionMap = this->m_staticCollisionMap](int val_){return pos.y >= m_staticCollisionMap[val_].y;});
                while (first != m_staticCollisionMapTop.end())
                {
                    if (m_pc.getPushbox().checkOverlapWith(m_staticCollisionMap[*first]))
                    {
                        pos.y = m_staticCollisionMap[*first].y;
                        vel.y = 0;
                    }
    
                    first++;
                }
            }
        }

        {
            pos.x += vel.x;
            if (vel.x > 0)
            {
                auto pb = m_pc.getPushbox();
                auto first = std::find_if(m_staticCollisionMapLeft.begin(), m_staticCollisionMapLeft.end(), [&pb, &m_staticCollisionMap = this->m_staticCollisionMap](int val_){return pb.x + pb.w >= m_staticCollisionMap[val_].x;});
                while (first != m_staticCollisionMapLeft.end())
                {
                    if (pb.checkOverlapWith(m_staticCollisionMap[*first]))
                    {
                        pos.x = m_staticCollisionMap[*first].x - pb.w / 2.0;
                        pb = m_pc.getPushbox();
                        vel.x = 0;
                    }

                    first++;
                }
            }
            else if (vel.x < 0)
            {
                auto pb = m_pc.getPushbox();
                auto first = std::find_if(m_staticCollisionMapRight.begin(), m_staticCollisionMapRight.end(), [&pb, &m_staticCollisionMap = this->m_staticCollisionMap](int val_){return pb.x <= m_staticCollisionMap[val_].x + m_staticCollisionMap[val_].w;});
                while (first != m_staticCollisionMapRight.end())
                {
                    if (pb.checkOverlapWith(m_staticCollisionMap[*first]))
                    {
                        pos.x = m_staticCollisionMap[*first].x + m_staticCollisionMap[*first].w + pb.w / 2.0;
                        pb = m_pc.getPushbox();
                        vel.x = 0;
                    }

                    first++;
                }
            }
        }

        m_camera.update();

        m_hud.update();

        //std::cout << m_characters[0]->CharStateData() << " | " << m_characters[1]->CharStateData() << std::endl;
    }

    void draw() override
    {
    	auto &renderer = *m_application->getRenderer();
    	renderer.fillRenderer(SDL_Color{ 60, 197, 255, 255 });

    	if (m_background.get())
    		m_background->draw(renderer, m_camera);

        for (const auto &cld : m_staticCollisionMap)
        {
            renderer.drawCollider(cld, {255, 0, 0, 100}, 100, m_camera);
        }

        m_pc.draw(m_camera);

        m_hud.draw(renderer, m_camera);

    	renderer.updateScreen();
    }

    void addStaticCollider(const Collider &cld_)
    {
        m_staticCollisionMap.push_back(cld_);
        m_staticCollisionMapTop.insert(m_staticCollisionMap.size() - 1);
    }

    HUD m_hud;
    Camera m_camera;
    std::unique_ptr<BackType> m_background;

    PlayableCharacter m_pc;

    std::vector<Collider> m_staticCollisionMap;
    std::set<int, std::function<bool(int, int)>> m_staticCollisionMapTop;
    std::set<int, std::function<bool(int, int)>> m_staticCollisionMapLeft;
    std::set<int, std::function<bool(int, int)>> m_staticCollisionMapRight;
};

#endif
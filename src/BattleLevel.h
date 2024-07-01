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
#include "DebugDataWidget.h"
#include "PlayableCharacter.h"

template <typename BackType>
class BattleLevel : public Level
{
public:
    BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_) :
    	Level(application_, size_, lvlId_),
    	m_camera({0.0f, 0.0f}, gamedata::global::baseResolution, m_size),
        m_pc(*application_, {380.0f, 230.0f})
    {
        static_assert(std::is_base_of_v<Background, BackType>, "BackType of BattleLevel should be derived from Background class");

        m_hud.addWidget(std::make_unique<DebugDataWidget>(*m_application, m_camera, lvlId_, size_));
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
    }

    virtual ~BattleLevel()
    {
    }

protected:
    void update() override
    {
        m_pc.update();

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

        m_pc.draw(m_camera);

        m_hud.draw(renderer, m_camera);

    	renderer.updateScreen();
    }

    HUD m_hud;
    Camera m_camera;
    std::unique_ptr<BackType> m_background;

    PlayableCharacter m_pc;
    
};

#endif
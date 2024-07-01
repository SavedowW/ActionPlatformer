#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "Window.h"
#include "Renderer.h"
#include "Level.h"
#include "InputSystem.h"
#include "TextureManager.h"
#include "AnimationManager.h"
#include <memory>
#include <filesystem>
#include "GameData.h"
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "TextManager.h"

class Application
{
public:
    Application();
    ~Application();

    Application& operator=(const Application &rhs) = delete;
    Application(const Application &rhs) = delete;

    Application& operator=(Application &&rhs);
    Application(Application &&rhs);

    void run();

    void setLevel(int levelId_, Level *level_);

    Renderer *getRenderer();
    InputSystem *getInputSystem();
    TextureManager *getTextureManager();
    AnimationManager *getAnimationManager();
    TextManager *getTextManager();

    std::string getBasePath();

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputSystem> m_inputSystem;
    std::unique_ptr<TextureManager> m_textureManager;
    std::unique_ptr<AnimationManager> m_animationManager;
    std::unique_ptr<TextManager> m_textManager;

    Level* m_levels[gamedata::global::numberOfLevels] {nullptr};
    LevelResult m_levelResult = {gamedata::global::initialLevelId};

    std::string m_appRoot;
};

#endif
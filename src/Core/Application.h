#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "Window.h"
#include "Renderer.h"
#include "Level.h"
#include "InputSystem.h"
#include "TextureManager.h"
#include "AnimationManager.h"
#include "TextManager.h"
#include <memory>
#include <SDL3_mixer/SDL_mixer.h>

namespace gamedata::levels
{
    inline constexpr int numberOfLevels = 2;
    inline constexpr int initialLevelId = 1;
}

class SDLCore
{
public:
    SDLCore();
    ~SDLCore();
};

class Application
{
private:
    SDLCore m_sdlCore;
    Window m_window;

public:
    static Application &instance();
    void run();

    void setLevel(int levelId_, std::unique_ptr<Level>&&);

    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application &operator=(const Application&) = delete;
    Application &operator=(Application&&) = delete;

    Renderer m_renderer;
    InputSystem m_inputSystem;
    TextureManager m_textureManager;
    AnimationManager m_animationManager;
    TextManager m_textManager;

private:
    Application();

    std::array<std::unique_ptr<Level>, gamedata::levels::numberOfLevels> m_levels{nullptr};
    LevelResult m_levelResult = {gamedata::levels::initialLevelId};
};

#endif
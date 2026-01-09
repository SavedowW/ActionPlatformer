#pragma once
#include "Window.h"
#include "Renderer.h"
#include "Level.h"
#include "InputSystem.h"
#include "TextureManager.h"
#include "AnimationManager.h"
#include "TextManager.h"
#include "FPSUtility.h"
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

    template<typename T, typename... Args>
    void makeLevel(int levelId_, Args&&... args_) 
        requires std::constructible_from<T, int, FPSUtility&, Args...>;

    const FPSUtility &getFPSUtility() const;

    void cycle();

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

    std::unordered_map<int, std::unique_ptr<Level>> m_levels;
    LevelResult m_levelResult = {gamedata::levels::initialLevelId};
    FPSUtility m_fpsUtility;
};

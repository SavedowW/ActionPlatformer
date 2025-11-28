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

    class TimeStep
    {
    public:
        TimeStep() noexcept;
        void processFrame() noexcept;
        Time::NS getFrameDuration() const noexcept;
        void applySlowdown() noexcept;
        void disableSlowdown() noexcept;
        void setForceDefault(bool forceDefault_) noexcept;


        /*
            Frame duration will not exceed this value - if fps drops too low, the game will slow down
            Also it's used for time calculations
        */
        constexpr static Time::NS defaultFrameDuration{1'000'000'000ull / 60};

    private:
        Timer m_timer;
        Time::NS m_lastFrameTime = defaultFrameDuration;
        double m_speedMultiplier = 1.0;
        bool m_forceDefault = false;
    } timestep;

private:
    Application();

    std::array<std::unique_ptr<Level>, gamedata::levels::numberOfLevels> m_levels;
    LevelResult m_levelResult = {gamedata::levels::initialLevelId};
};

#endif
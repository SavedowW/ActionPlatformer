#include "Application.h"
#include "FilesystemUtils.h"
#include "Localization/LocalizationGen.h"
#include "SDL3/SDL_error.h"
#include "Logger.hpp"

Application &Application::instance()
{
    static Application app;
    return app;
}

SDLCore::SDLCore()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        throw std::runtime_error(std::string("SDL initialization error: ") + SDL_GetError());

    if (!TTF_Init())
        throw std::runtime_error(std::string("TTF initialization error: ") + SDL_GetError());

    if (SDL_Init(SDL_INIT_AUDIO))
    {
        if (!MIX_Init())
        {
            LOG_INFO("Audio system was enabled successfully!");
        }
        else
        {
            LOG_ERROR("MIX initialization error: {}", SDL_GetError());
        }
    }
    else
    {
        LOG_ERROR("SDL_INIT_AUDIO initialization error: {}", SDL_GetError());
    }

    Filesystem::ensureDirectoryRelative("Resources");
    Filesystem::ensureDirectoryRelative("Resources/Fonts");

    ll::load();
    ll::setLang("en");
}

SDLCore::~SDLCore()
{
    MIX_Quit();
    TTF_Quit();
    SDL_Quit();
    LOG_INFO("Application shut down successfully");
}

const FPSUtility &Application::getFPSUtility() const
{
    return m_fpsUtility;
}

Application::Application() :
    m_window("GameName"),
    m_renderer(m_window),
    m_textManager(m_renderer),
    m_fpsUtility{60}
{
    Filesystem::ensureDirectoryRelative("Tilemaps");
    Filesystem::ensureDirectoryRelative("Configs");
}

void Application::run()
{
    m_fpsUtility.start();
    while (m_levelResult.nextLvl.has_value())
    {
        m_levels[*m_levelResult.nextLvl]->enter();
        m_levelResult = m_levels[*m_levelResult.nextLvl]->proceed();
    }
}

#include "Application.h"
#include "FilesystemUtils.h"
#include "Localization/LocalizationGen.h"
#include "SDL3/SDL_error.h"

Application &Application::instance()
{
    static Application app;
    return app;
}

SDLCore::SDLCore()
{
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        throw std::runtime_error(std::string("SDL initialization error: ") + SDL_GetError());

    if (!SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 ))
        throw std::runtime_error(std::string("Error setting SDL_GL_CONTEXT_MAJOR_VERSION: ") + SDL_GetError());

    if (!SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 ))
        throw std::runtime_error(std::string("Error setting SDL_GL_CONTEXT_MAJOR_VERSION: ") + SDL_GetError());

    if (!SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE ))
        throw std::runtime_error(std::string("Error setting SDL_GL_CONTEXT_PROFILE_MASK: ") + SDL_GetError());


    if (!TTF_Init())
        throw std::runtime_error(std::string("TTF initialization error: ") + SDL_GetError());

    if (!MIX_Init())
        throw std::runtime_error(std::string("MIX initialization error: ") + SDL_GetError());

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
    std::cout << "Application shut down successfully" << std::endl;
}

Application::Application() :
    m_window("GameName"),
    m_renderer(m_window.getWindow()),
    m_textManager(m_renderer)
{
    Filesystem::ensureDirectoryRelative("Tilemaps");
    Filesystem::ensureDirectoryRelative("Configs");
}

void Application::run()
{
    while (m_levelResult.nextLvl != -1)
    {
        m_levels[m_levelResult.nextLvl]->enter();
        m_levelResult = m_levels[m_levelResult.nextLvl]->proceed();
    }
}

void Application::setLevel(int levelId_, std::unique_ptr<Level> &&level_)
{
    m_levels[levelId_] = std::move(level_);
}

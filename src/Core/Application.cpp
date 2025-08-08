#include "Application.h"
#include "FilesystemUtils.h"
#include "Localization/LocalizationGen.h"

Application::Application()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "SDL initialization error: " << SDL_GetError() << std::endl;
		throw std::runtime_error("Cannot initialize SDL");
	}
    
    if (SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 ) < 0)
        std::cout << SDL_GetError() << std::endl;
    if (SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 ) < 0)
        std::cout << SDL_GetError() << std::endl;
    if (SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE ) < 0)
        std::cout << SDL_GetError() << std::endl;

    if (TTF_Init() == -1)
	{
		std::cout << "TTF initialization error: " << TTF_GetError() << std::endl;
	}

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 0, 512))
	{
		std::cout << "MIX ititialization error: " << Mix_GetError() << std::endl;
	}

    Filesystem::ensureDirectoryRelative("Resources");
	Filesystem::ensureDirectoryRelative("Resources/Animations");
	Filesystem::ensureDirectoryRelative("Resources/Sprites");
	Filesystem::ensureDirectoryRelative("Resources/Fonts");
	Filesystem::ensureDirectoryRelative("Tilemaps");
	Filesystem::ensureDirectoryRelative("Configs");

    ll::load();
    ll::setLang("en");

    m_window = std::make_unique<Window>("GameName");
    m_renderer = std::make_unique<Renderer>(m_window->getWindow());
    m_inputSystem = std::make_unique<InputSystem>();
    m_textureManager = std::make_unique<TextureManager>(*m_renderer);
    m_animationManager = std::make_unique<AnimationManager>(*m_renderer);
    m_textManager = std::make_unique<TextManager>(*m_renderer);

    m_inputSystem->initiateControllers();
}

Application::~Application()
{
    m_textManager.reset();
    m_animationManager.reset();
    m_textureManager.reset();
    m_inputSystem.reset();
    m_renderer.reset();
    m_window.reset();
    Mix_Quit();
    TTF_Quit();
	SDL_Quit();
    std::cout << "Application shut down successfully" << std::endl;
}

void Application::run()
{
    while (m_levelResult.nextLvl != -1)
	{
		m_levels[m_levelResult.nextLvl]->enter();
		m_levelResult = m_levels[m_levelResult.nextLvl]->proceed();
	}
}

void Application::setLevel(int levelId_, Level *level_)
{
    if (m_levels[levelId_])
        delete m_levels[levelId_];
    m_levels[levelId_] = level_;
}

Renderer &Application::getRenderer()
{
    return *m_renderer;
}

InputSystem &Application::getInputSystem()
{
    return *m_inputSystem;
}

TextureManager &Application::getTextureManager()
{
    return *m_textureManager;
}

AnimationManager &Application::getAnimationManager()
{
    return *m_animationManager;
}

TextManager &Application::getTextManager()
{
    return *m_textManager;
}

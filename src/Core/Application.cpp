#include "Application.h"

Application::Application()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "SDL initialization error: " << SDL_GetError() << std::endl;
		throw std::runtime_error("Cannot initialize SDL");
	}

	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
	{
		std::cout << "IMG initialization error: " << SDL_GetError() << std::endl;
		throw std::runtime_error("Cannot initialize SDL image");
	}

    if (TTF_Init() == -1)
	{
		std::cout << "TTF initialization error: " << TTF_GetError() << std::endl;
	}

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 0, 512))
	{
		std::cout << "MIX ititialization error: " << Mix_GetError() << std::endl;
	}


    m_appRoot = getBasePath();

    m_window = std::make_unique<Window>(Vector2{1600.0f, 900.0f}, "GameName");
    m_renderer = std::make_unique<Renderer>(*m_window.get());
    m_inputSystem = std::make_unique<InputSystem>();
    m_textureManager = std::make_unique<TextureManager>(m_renderer.get(), m_appRoot);
    m_animationManager = std::make_unique<AnimationManager>(m_renderer.get(), m_appRoot);
    m_textManager = std::make_unique<TextManager>(m_renderer.get(), m_appRoot);
}

Application::~Application()
{
    m_animationManager.reset();
    m_textureManager.reset();
    m_inputSystem.reset();
    m_renderer.reset();
    m_window.reset();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
	SDL_Quit();
}

Application& Application::operator=(Application &&rhs)
{
    // TODO

    throw std::runtime_error("Not implemented yet");

    return *this;
}

Application::Application(Application &&rhs)
{
    // TODO

    throw std::runtime_error("Not implemented yet");
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

Renderer *Application::getRenderer()
{
    return m_renderer.get();
}

InputSystem *Application::getInputSystem()
{
    return m_inputSystem.get();
}

TextureManager *Application::getTextureManager()
{
    return m_textureManager.get();
}

AnimationManager *Application::getAnimationManager()
{
    return m_animationManager.get();
}

TextManager *Application::getTextManager()
{
    return m_textManager.get();
}

std::string Application::getBasePath()
{
    auto pathptr = SDL_GetBasePath();
    if (!pathptr)
        return "";

    std::string path = pathptr;
    SDL_free(pathptr);
    std::filesystem::path ppath(path);
    
    for (; ppath.filename() != "build" && !ppath.empty(); ppath = ppath.parent_path());

    return ppath.parent_path().string();
}
#include "Level.h"
#include "Profile.h"
#include <optional>

Level::Level(std::string levelName_, FPSUtility &fpsUtility_, const Vector2<int> &size_) :
    m_levelName{std::move(levelName_)},
    m_fpsUtility{fpsUtility_},
    m_size{size_}
{
    subscribe(GAMEPLAY_EVENTS::QUIT);
    subscribe(GAMEPLAY_EVENTS::FN3);
    subscribe(GAMEPLAY_EVENTS::FN2);
    subscribe(GAMEPLAY_EVENTS::FN1);

    setInputDisabled();
}

//Runs when enter the function
void Level::enter()
{
    m_state = STATE::RUNNING;
    m_returnVal.nextLvl.reset();
    setInputEnabled();
}

void Level::leave()
{
    setInputDisabled();
}

LevelResult Level::proceed()
{
    Timer fullFrameTime;
    auto &profiler = Profiler::instance();

    fullFrameTime.begin();
    while (m_state == STATE::RUNNING)
    {
        profiler.cleanFrame();
        m_input.handleInput();

        const auto iterateCon = !m_globalPause || m_globalPause && m_allowIter || m_forcerun;

        if (iterateCon)
        {
            update();
            m_allowIter = false;
        }

        draw();
        #ifdef DUMP_PROFILE_CONSOLE
        if (iterateCon)
        {
            profiler.dump();
            std::cout << std::endl;
        }
        #endif

        m_fpsUtility.cycle();
    }

    leave();

    return m_returnVal;
}

std::string Level::name() const
{
    return m_levelName;
}

void Level::receiveEvents(GAMEPLAY_EVENTS event, const float scale_)
{
    switch (event)
    {
        case (GAMEPLAY_EVENTS::QUIT):
            m_returnVal.nextLvl.reset();
            m_state = STATE::LEAVE;
            break;

        case (GAMEPLAY_EVENTS::FN3):
            if (scale_ > 0)
                m_globalPause = !m_globalPause;
            break;

        case (GAMEPLAY_EVENTS::FN2):
            if (scale_ > 0)
                m_allowIter = true;
            break;

        case (GAMEPLAY_EVENTS::FN1):
            if (scale_ > 0)
            {
                m_fpsUtility.setFPS(20);
                m_forcerun = true;
            }
            else
            {
                m_fpsUtility.setDefaultFPS();
                m_forcerun = false;
            }
            break;
        
        default:
            break;
    }
}

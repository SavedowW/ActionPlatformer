#include "Level.h"
#include "Profile.h"
#include "GameData.h"

#if 0
static std::string nicetime(const uint64_t &ns_)
{
    return std::to_string(static_cast<float>(ns_.count()) / 1000000.0f);
}
#endif

Level::Level(int lvlId_, FPSUtility &fpsUtility_, const Vector2<int> &size_) :
    m_size{size_},
    m_levelId{lvlId_},
    m_fpsUtility{fpsUtility_}
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
    m_returnVal = { -1 };
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

        if (!m_globalPause || m_globalPause && m_allowIter || m_forcerun)
        {
            update();
            m_allowIter = false;
        }

        draw();
        #ifdef DUMP_PROFILE_CONSOLE
            profiler.dump();
            std::cout << std::endl;
        #endif

        m_fpsUtility.cycle();
    }

    leave();

    return m_returnVal;
}

void Level::receiveEvents(GAMEPLAY_EVENTS event, const float scale_)
{
    switch (event)
    {
        case (GAMEPLAY_EVENTS::QUIT):
            m_returnVal = { -1 };
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
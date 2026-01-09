#include "Level.h"
#include "Profile.h"
#include "GameData.h"

#if 0
static std::string nicetime(const uint64_t &ns_)
{
    return std::to_string(static_cast<float>(ns_.count()) / 1000000.0f);
}
#endif

Level::Level(const Vector2<int> &size_, int lvlId_) :
    m_size(size_),
    m_levelId(lvlId_),
    m_timeForFrame(1'000'000'000 / gamedata::global::framerate),
    m_lastFrameTimeNS{0}
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
    m_frameTimer.begin();
    setInputEnabled();
}

void Level::leave()
{
    setInputDisabled();
}

LevelResult Level::proceed()
{
    m_frameTimer.begin();
    uint64_t compensate = 0;

    Timer fullFrameTime;
    auto &profiler = Profiler::instance();

    fullFrameTime.begin();
    m_frameTimer.begin();
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

        m_lastFrameTimeNS = m_frameTimer.getPassed();
        if (m_lastFrameTimeNS < m_timeForFrame + compensate)
        {
            SDL_DelayNS(m_timeForFrame - m_lastFrameTimeNS + compensate);
            compensate = m_timeForFrame - m_frameTimer.getPassed();
        }
        else
            compensate += m_timeForFrame - m_lastFrameTimeNS;
        m_frameTimer.begin();

        m_lastFullFrameTime = fullFrameTime.getPassed();
        fullFrameTime.begin();
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
                m_timeForFrame = 1'000'000'000 / gamedata::global::dbgslowdownfps;
                m_forcerun = true;
            }
            else
            {
                m_timeForFrame = 1'000'000'000 / gamedata::global::framerate;
                m_forcerun = false;
            }
            break;
        
        default:
            break;
    }
}
#include "Level.h"
#include "Application.h"
#include "Profile.h"
#include "GameData.h"
#include <thread>

static std::string nicetime(const nanoseconds &ns_)
{
	return std::to_string(static_cast<float>(ns_.count()) / 1000000.0f);
}

Level::Level(Application &application_, const Vector2<int> &size_, int lvlId_) :
	InputReactor(application_.getInputSystem()),
	m_size(size_),
	m_levelId(lvlId_),
	m_application(application_),
	m_timeForFrame(nanoseconds(1000000000) / static_cast<long long>(gamedata::global::framerate)),
	m_lastFrameTimeMS{0}
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
	nanoseconds compensate = nanoseconds(0);

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
			draw();
			m_allowIter = false;

			#ifdef DUMP_PROFILE_CONSOLE
				profiler.dump();
				std::cout << std::endl;
			#endif
		}

		m_lastFrameTimeMS = m_frameTimer.getPassed<nanoseconds>();
		if (m_lastFrameTimeMS < m_timeForFrame + compensate)
		{
			std::this_thread::sleep_for(m_timeForFrame - m_lastFrameTimeMS + compensate);
			compensate = m_timeForFrame - m_frameTimer.getPassed<nanoseconds>();
		}
		else
			compensate += m_timeForFrame - m_lastFrameTimeMS;
		m_frameTimer.begin();

		m_lastFullFrameTime = fullFrameTime.getPassed<nanoseconds>();
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
				m_timeForFrame = nanoseconds(1000000000) / static_cast<long long>(gamedata::global::dbgslowdownfps);
				m_forcerun = true;
			}
			else
			{
				m_timeForFrame = nanoseconds(1000000000) / static_cast<long long>(gamedata::global::framerate);
				m_forcerun = false;
			}
			break;
	}
}
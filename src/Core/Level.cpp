#include "Level.h"
#include "Application.h"
#include <thread>

Level::Level(Application *application_, const Vector2<float> &size_, int lvlId_) :
	InputReactor(application_->getInputSystem()),
	m_size(size_),
	m_levelId(lvlId_),
	m_application(application_),
	m_timeForFrame(nanoseconds(1000000000) / static_cast<long long>(gamedata::global::framerate)),
	m_lastFrameTimeMS{0}
{
	subscribe(EVENTS::QUIT);
	subscribe(EVENTS::FN3);
	subscribe(EVENTS::FN2);
	subscribe(EVENTS::FN1);

	setInputEnabled(false);
}

//Runs when enter the function
void Level::enter()
{
	m_state = STATE::RUNNING;
	m_returnVal = { -1 };
	m_frameTimer.begin();
	setInputEnabled(true);
}

void Level::leave()
{
	setInputEnabled(false);
}

LevelResult Level::proceed()
{
	m_frameTimer.begin();
	nanoseconds compensate = nanoseconds(0);

	Timer fullFrameTime;

	while (m_state == STATE::RUNNING)
	{
		fullFrameTime.begin();

		m_input->handleInput();

		if (!m_globalPause || m_globalPause && m_allowIter || m_forcerun)
		{
			update();
			draw();
			m_allowIter = false;
		}

		m_lastFrameTimeMS = m_frameTimer.getPassedNano();
		if (m_lastFrameTimeMS < m_timeForFrame + compensate)
		{
			nanoseconds nanoToSleep = m_timeForFrame - m_lastFrameTimeMS + compensate;
			m_frameTimer.begin();
			std::this_thread::sleep_for(nanoToSleep);
			compensate = nanoToSleep - m_frameTimer.getPassedNano();
		}
		m_frameTimer.begin();

		m_lastFullFrameTime = fullFrameTime.getPassedNano();
	}

	leave();

	return m_returnVal;
}

void Level::receiveInput(EVENTS event, const float scale_)
{
	switch (event)
	{		
		case (EVENTS::QUIT):
			m_returnVal = { -1 };
			m_state = STATE::LEAVE;
			break;

		case (EVENTS::FN3):
			if (scale_ > 0)
				m_globalPause = !m_globalPause;
			break;

		case (EVENTS::FN2):
			if (scale_ > 0)
				m_allowIter = true;
			break;

		case (EVENTS::FN1):
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
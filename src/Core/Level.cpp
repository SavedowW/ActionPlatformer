#include "Level.h"
#include "Application.h"

Level::Level(Application *application_, const Vector2<float> &size_, int lvlId_) :
	InputReactor(application_->getInputSystem()),
	m_size(size_),
	m_levelId(lvlId_),
	m_application(application_),
	m_timeForFrame(1000.0f / gamedata::global::framerate)
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
	float compensate = 0;

	while (m_state == STATE::RUNNING)
	{
		//system("cls");
		m_input->handleInput();

		if (!m_globalPause || m_globalPause && m_allowIter || m_forcerun)
		{
			update();
			draw();
			m_allowIter = false;
		}
		//destroyRequested();

		m_lastFrameTimeMS = m_frameTimer.getPassedMS();
		if (m_lastFrameTimeMS < m_timeForFrame + compensate)
		{
			float fTimeToSleep = m_timeForFrame - m_lastFrameTimeMS + compensate;
			int iTimeToSleep = fTimeToSleep;
			compensate = fTimeToSleep - iTimeToSleep;
			SDL_Delay(iTimeToSleep);
		}
		m_frameTimer.begin();
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
				m_timeForFrame = 1000.0f / gamedata::global::dbgslowdownfps;
				m_forcerun = true;
			}
			else
			{
				m_timeForFrame = 1000.0f / gamedata::global::framerate;
				m_forcerun = false;
			}
			break;
	}
}
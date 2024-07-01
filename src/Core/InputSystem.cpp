#include "InputSystem.h"
#include <iostream>

InputSystem::InputSystem()
{
}

void InputSystem::handleInput()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case (SDL_QUIT):
			send(EVENTS::QUIT, 1);
			break;

		case (SDL_KEYDOWN):
		{
			if (e.key.repeat)
				break;
			auto res = KEY_EVENTS.find(e.key.keysym.sym);
			if (res != KEY_EVENTS.end())
			{
				send(res->second, 1);
			}
			
		}
			break;

		case (SDL_KEYUP):
		{
			auto res = KEY_EVENTS.find(e.key.keysym.sym);
			if (res != KEY_EVENTS.end())
			{
				send(res->second, -1);
			}
		}
		break;
		}
	}
}

//must be called by InputReactor
void InputSystem::subscribe(EVENTS ev_, subscriber sub_)
{
	if (!sub_)
		throw "Input reactor is nullptr";

	m_subscribers[(int)ev_].push_back(sub_);
}

//Automatically called when InputReactor is destroyed
void InputSystem::unsubscribe(EVENTS ev_, subscriber sub_)
{
	std::vector<subscriber>& v = m_subscribers[(int)ev_];
}

void InputSystem::send(EVENTS ev_, float val_)
{
	for (subscriber& sub : m_subscribers[(int)ev_])
	{
		if (sub->isInputEnabled())
		{
			sub->receiveInput(ev_, val_);
		}
	}
}

void InputReactor::receiveInput(EVENTS event, const float scale_)
{
}

void InputReactor::setInputEnabled(bool inputEnabled_)
{
	m_inputEnabled = inputEnabled_;
}

bool InputReactor::isInputEnabled()
{
	return m_inputEnabled;
}

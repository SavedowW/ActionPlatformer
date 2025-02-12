#ifndef INPUT_H_
#define INPUT_H_
#include <SDL.h>
#include <SDL_Image.h>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <set>

//List of possible events
//Anything that is inherited from InputReactor can subscribe to them
enum class EVENTS
{
	QUIT,
	UP,
	RIGHT,
	DOWN,
	LEFT,
	ATTACK,
	FN1,
	FN2,
	FN3,
	FN4,
	CAM_STOP,
	REN_DBG_1,
	NONE
};

class InputReactor;

typedef InputReactor* subscriber;

class InputSystem
{
public:
	InputSystem();
	void handleInput();
	void subscribe(EVENTS ev_, subscriber sub_);
	void unsubscribe(EVENTS ev_, subscriber sub_);

private:
	void updateAxis(EVENTS ev_, float scale_);
	void send(EVENTS ev_, float val_);
	std::vector<subscriber> m_subscribers[(int)EVENTS::NONE];
	std::map<SDL_Keycode, EVENTS> KEY_EVENTS = {
		{SDLK_w, EVENTS::UP},
		{SDLK_d, EVENTS::RIGHT},
		{SDLK_s, EVENTS::DOWN},
		{SDLK_a, EVENTS::LEFT},
		{SDLK_i, EVENTS::ATTACK},
		{SDLK_MINUS, EVENTS::FN1},
		{SDLK_EQUALS, EVENTS::FN2},
		{SDLK_0, EVENTS::FN3},
		{SDLK_SPACE, EVENTS::CAM_STOP},
		{SDLK_BACKSPACE, EVENTS::FN4},
		{SDLK_SLASH, EVENTS::REN_DBG_1}
	};
};


class InputReactor
{
public:
	//Input reactor needs pointer to  InputSystem
	InputReactor(InputSystem *input_) :
		m_input(input_)
	{
	}

	virtual void receiveInput(EVENTS event, const float scale_);

	//All events reactor is subscribed at
	std::set<EVENTS> subscribed_at;

	void setInputEnabled(bool inputEnabled_);
	bool isInputEnabled();

	//InputReactor automatically removes itself from subscribers
	virtual ~InputReactor()
	{
		while(!subscribed_at.empty())
		{
			unsubscribe(*subscribed_at.begin());
		}
	}

protected:
	void subscribe(EVENTS ev_)
	{
		if (subscribed_at.contains(ev_))
			return;
		
		m_input->subscribe(ev_, this);
		subscribed_at.insert(ev_);
	}

	void unsubscribe(EVENTS ev_)
	{
		if (!subscribed_at.contains(ev_))
			return;
		
		m_input->unsubscribe(ev_, this);
		subscribed_at.erase(ev_);
	}

	void unsubscribeFromAll()
	{
		while(!subscribed_at.empty())
		{
			unsubscribe(*subscribed_at.begin());
		}
	}

	InputSystem * m_input;
	bool m_inputEnabled = false;

};

#endif
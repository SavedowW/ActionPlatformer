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

struct ControllerDescription
{
    SDL_GameController *m_controller;
};

class InputSystem
{
public:
    InputSystem();
    void handleInput();
    void subscribe(EVENTS ev_, subscriber sub_);
    void unsubscribe(EVENTS ev_, subscriber sub_);

    void initiateControllers();

private:
    void send(EVENTS ev_, float val_);
    std::vector<subscriber> m_subscribers[(int)EVENTS::NONE];

    std::map<SDL_Keycode, EVENTS> m_keyboardBindings = {
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

    std::map<Uint8, EVENTS> m_gamepadBindings = {
        {SDL_CONTROLLER_BUTTON_A, EVENTS::UP},
        {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, EVENTS::RIGHT},
        {SDL_CONTROLLER_BUTTON_DPAD_DOWN, EVENTS::DOWN},
        {SDL_CONTROLLER_BUTTON_DPAD_LEFT, EVENTS::LEFT},
        {SDL_CONTROLLER_BUTTON_X, EVENTS::ATTACK},
        {SDL_CONTROLLER_BUTTON_RIGHTSTICK, EVENTS::FN1},
        {SDL_CONTROLLER_BUTTON_START, EVENTS::FN2},
        {SDL_CONTROLLER_BUTTON_BACK, EVENTS::FN3},
        {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, EVENTS::CAM_STOP},
        {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, EVENTS::FN4},
        {SDL_CONTROLLER_BUTTON_LEFTSTICK, EVENTS::REN_DBG_1}
    };

    std::map<Uint8, EVENTS> m_gamepadPositiveAxisBindings = {
        {SDL_CONTROLLER_AXIS_LEFTX, EVENTS::RIGHT},
        {SDL_CONTROLLER_AXIS_LEFTY, EVENTS::DOWN}
    };

    std::map<Uint8, EVENTS> m_gamepadNegativeAxisBindings = {
        {SDL_CONTROLLER_AXIS_LEFTX, EVENTS::LEFT},
        {SDL_CONTROLLER_AXIS_LEFTY, EVENTS::UP}
    };

    std::map<Uint8, Sint16> m_lastAxisValue = {
        {SDL_CONTROLLER_AXIS_LEFTX, 0},
        {SDL_CONTROLLER_AXIS_LEFTY, 0},
        {SDL_CONTROLLER_AXIS_RIGHTX, 0},
        {SDL_CONTROLLER_AXIS_RIGHTY, 0}
    };

    std::map<int, ControllerDescription> m_controllers;

    const Sint16 m_stickDeadzone = 16000;
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
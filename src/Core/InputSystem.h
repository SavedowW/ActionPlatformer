#ifndef INPUT_H_
#define INPUT_H_
#include <SDL.h>
#include <SDL_Image.h>
#include "EnumMapping.hpp"
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <set>

//List of possible gameplay events
//Anything that is inherited from InputReactor can subscribe to them
enum class GAMEPLAY_EVENTS
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

// Events used for menus and hud in general
enum class HUD_EVENTS
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    PROCEED,
    NONE
};

SERIALIZE_ENUM(SDL_GameControllerButton, {
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_INVALID, "INVALID"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_A, "A"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_B, "B"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_X, "X"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_Y, "Y"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_BACK, "BACK"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_GUIDE, "GUIDE"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_START, "START"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_LEFTSTICK, "LEFTSTICK"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_RIGHTSTICK, "RIGHTSTICK"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "LEFTSHOULDER"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, "RIGHTSHOULDER"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_DPAD_UP, "DPAD_UP"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_DPAD_DOWN, "DPAD_DOWN"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_DPAD_LEFT, "DPAD_LEFT"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, "DPAD_RIGHT"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_MISC1, "MISC1"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_PADDLE1, "PADDLE1"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_PADDLE2, "PADDLE2"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_PADDLE3, "PADDLE3"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_PADDLE4, "PADDLE4"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_TOUCHPAD, "TOUCHPAD"),
    ENUM_INIT(SDL_GameControllerButton, SDL_CONTROLLER_BUTTON_MAX, "MAX")
})

SERIALIZE_ENUM(SDL_GameControllerAxis, {
    ENUM_INIT(SDL_GameControllerAxis, SDL_CONTROLLER_AXIS_INVALID, "INVALID"),
    ENUM_INIT(SDL_GameControllerAxis, SDL_CONTROLLER_AXIS_LEFTX, "LEFTX"),
    ENUM_INIT(SDL_GameControllerAxis, SDL_CONTROLLER_AXIS_LEFTY, "LEFTY"),
    ENUM_INIT(SDL_GameControllerAxis, SDL_CONTROLLER_AXIS_RIGHTX, "RIGHTX"),
    ENUM_INIT(SDL_GameControllerAxis, SDL_CONTROLLER_AXIS_RIGHTY, "RIGHTY"),
    ENUM_INIT(SDL_GameControllerAxis, SDL_CONTROLLER_AXIS_TRIGGERLEFT, "TRIGGERLEFT"),
    ENUM_INIT(SDL_GameControllerAxis, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, "TRIGGERRIGHT"),
    ENUM_INIT(SDL_GameControllerAxis, SDL_CONTROLLER_AXIS_MAX, "MAX")
})

template<typename EventT>
struct EventBinding
{
    std::map<SDL_Keycode, EventT> m_keyboardBindings;
    std::map<SDL_GameControllerButton, EventT> m_gamepadBindings;
    std::map<SDL_GameControllerAxis, EventT> m_gamepadPositiveAxisBindings;
    std::map<SDL_GameControllerAxis, EventT> m_gamepadNegativeAxisBindings;
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
    void subscribe(GAMEPLAY_EVENTS ev_, subscriber sub_);
    void unsubscribe(GAMEPLAY_EVENTS ev_, subscriber sub_);
    void subscribe(HUD_EVENTS ev_, subscriber sub_);
    void unsubscribe(HUD_EVENTS ev_, subscriber sub_);

    void initiateControllers();


private:
    void send(GAMEPLAY_EVENTS ev_, float val_);
    void send(HUD_EVENTS ev_, float val_);
    std::vector<subscriber> m_gameplaySubscribers[(int)GAMEPLAY_EVENTS::NONE];
    std::vector<subscriber> m_hudSubscribers[(int)HUD_EVENTS::NONE];

    template<typename InputT, typename ButtonT, typename EventT>
    void resolveBinding(const std::map<InputT, EventT> &bindings_, const ButtonT &input_, float value_);

    void setupDefaultMapping();

    EventBinding<GAMEPLAY_EVENTS> m_gameplayBindings;
    EventBinding<HUD_EVENTS> m_hudBindings;

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
    InputReactor(InputSystem &input_);

    virtual void receiveEvents(GAMEPLAY_EVENTS event, const float scale_);
    virtual void receiveEvents(HUD_EVENTS event, const float scale_);

    void setInputEnabled();
    void setInputDisabled();
    bool isInputEnabled();

    //InputReactor automatically removes itself from subscribers
    virtual ~InputReactor();

protected:
    void subscribe(GAMEPLAY_EVENTS ev_);
    void subscribe(HUD_EVENTS ev_);
    void unsubscribe(GAMEPLAY_EVENTS ev_);
    void unsubscribe(HUD_EVENTS ev_);
    void unsubscribeFromAll();

    InputSystem &m_input;
    bool m_inputEnabled = false;

    //All GAMEPLAY_EVENTS reactor is subscribed at
    std::set<GAMEPLAY_EVENTS> m_gameplaySubscriptions;
    std::set<HUD_EVENTS> m_hudSubscriptions;

};

#endif

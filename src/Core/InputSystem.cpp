#include "InputSystem.h"
#include <iostream>

InputSystem::InputSystem()
{
    setupDefaultMapping();
}

void InputSystem::handleInput()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case (SDL_QUIT):
            send(GAMEPLAY_EVENTS::QUIT, 1);
            break;

        case (SDL_KEYDOWN):
        {
            if (e.key.repeat)
                break;
            resolveBinding(m_gameplayBindings.m_keyboardBindings, e.key.keysym.sym, 1);
            resolveBinding(m_hudBindings.m_keyboardBindings, e.key.keysym.sym, 1);
            
        }
            break;

        case (SDL_KEYUP):
        {
            resolveBinding(m_gameplayBindings.m_keyboardBindings, e.key.keysym.sym, -1);
            resolveBinding(m_hudBindings.m_keyboardBindings, e.key.keysym.sym, -1);
        }
        break;

        case (SDL_CONTROLLERDEVICEADDED):
            m_controllers[e.cdevice.which].m_controller = SDL_GameControllerOpen(e.cdevice.which);
            std::cout << "Discovered new controller at " << e.cdevice.which << ": " << SDL_GameControllerName(m_controllers[e.cdevice.which].m_controller) << std::endl;
        break;

        case (SDL_CONTROLLERDEVICEREMOVED):
            std::cout << "Removing controller at " << e.cdevice.which << std::endl;
            SDL_GameControllerClose(m_controllers[e.cdevice.which].m_controller);
            m_controllers.erase(e.cdevice.which);
        break;

        case (SDL_CONTROLLERBUTTONDOWN):
        {
            //std::cout << "Controller pressed " << gamepadButtonNames.at(e.cbutton.button) << std::endl;

            resolveBinding(m_gameplayBindings.m_gamepadBindings, e.cbutton.button, 1);
            resolveBinding(m_hudBindings.m_gamepadBindings, e.cbutton.button, 1);
        }
        break;

        case (SDL_CONTROLLERBUTTONUP):
        {
            //std::cout << "Controller released " << gamepadButtonNames.at(e.cbutton.button) << std::endl;

            resolveBinding(m_gameplayBindings.m_gamepadBindings, e.cbutton.button, -1);
            resolveBinding(m_hudBindings.m_gamepadBindings, e.cbutton.button, -1);
        }
        break;

        case (SDL_CONTROLLERAXISMOTION):
        {
            auto resolvedValue = e.caxis.value;
            if (abs(resolvedValue) < m_stickDeadzone)
                resolvedValue = 0;

            auto lastValueRes = m_lastAxisValue.find(e.caxis.axis);
            if (lastValueRes != m_lastAxisValue.end())
            {
                if (lastValueRes->second == resolvedValue)
                    break;

                lastValueRes->second = resolvedValue;
            }

            float posValue, negValue;
            if (resolvedValue >= 0)
            {
                posValue = resolvedValue / 32767.0f;
                negValue = 0;
            }
            else
            {
                negValue = resolvedValue / -32768.0f;
                posValue = 0;
            }

            resolveBinding(m_gameplayBindings.m_gamepadPositiveAxisBindings, e.caxis.axis, posValue);
            resolveBinding(m_gameplayBindings.m_gamepadNegativeAxisBindings, e.caxis.axis, negValue);
            resolveBinding(m_hudBindings.m_gamepadPositiveAxisBindings, e.caxis.axis, posValue);
            resolveBinding(m_hudBindings.m_gamepadNegativeAxisBindings, e.caxis.axis, negValue);

            //std::cout << "Controller axis motion " << gamepadAxisNames.at(e.caxis.axis) << " : " << int(e.caxis.value) << std::endl;
        }
        break;

        default:
            //std::cout << "Unknown event " << int(e.type) << std::endl;
        break;
        
        }
    }
}

//must be called by InputReactor
void InputSystem::subscribe(GAMEPLAY_EVENTS ev_, subscriber sub_)
{
    if (!sub_)
        throw "Input reactor is nullptr";

    m_gameplaySubscribers[(int)ev_].push_back(sub_);
}

//Automatically called when InputReactor is destroyed
void InputSystem::unsubscribe(GAMEPLAY_EVENTS ev_, subscriber sub_)
{
    std::vector<subscriber>& v = m_gameplaySubscribers[(int)ev_];
}

void InputSystem::subscribe(HUD_EVENTS ev_, subscriber sub_)
{
    if (!sub_)
        throw "Input reactor is nullptr";

    m_hudSubscribers[(int)ev_].push_back(sub_);
}

void InputSystem::unsubscribe(HUD_EVENTS ev_, subscriber sub_)
{
    std::vector<subscriber>& v = m_hudSubscribers[(int)ev_];
}

void InputSystem::initiateControllers()
{
    std::cout << "Discovering controllers..." << std::endl;
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            auto &ctrl = m_controllers[i];
            ctrl.m_controller = SDL_GameControllerOpen(i);
            std::cout << "Found supported controller at " << i << ": " << SDL_GameControllerName(ctrl.m_controller) << std::endl;
        }
        else
        {
            std::cout << "Found unknown joystick at " << i << std::endl;
        }
    }
}

void InputSystem::send(GAMEPLAY_EVENTS ev_, float val_)
{
    for (subscriber& sub : m_gameplaySubscribers[(int)ev_])
    {
        if (sub->isInputEnabled())
        {
            sub->receiveEvents(ev_, val_);
        }
    }
}

void InputSystem::send(HUD_EVENTS ev_, float val_)
{
    for (subscriber& sub : m_hudSubscribers[(int)ev_])
    {
        if (sub->isInputEnabled())
        {
            sub->receiveEvents(ev_, val_);
        }
    }
}

void InputSystem::setupDefaultMapping()
{
    m_gameplayBindings.m_keyboardBindings = {
        {SDLK_w, GAMEPLAY_EVENTS::UP},
        {SDLK_d, GAMEPLAY_EVENTS::RIGHT},
        {SDLK_s, GAMEPLAY_EVENTS::DOWN},
        {SDLK_a, GAMEPLAY_EVENTS::LEFT},
        {SDLK_i, GAMEPLAY_EVENTS::ATTACK},
        {SDLK_MINUS, GAMEPLAY_EVENTS::FN1},
        {SDLK_EQUALS, GAMEPLAY_EVENTS::FN2},
        {SDLK_0, GAMEPLAY_EVENTS::FN3},
        {SDLK_SPACE, GAMEPLAY_EVENTS::CAM_STOP},
        {SDLK_BACKSPACE, GAMEPLAY_EVENTS::FN4},
        {SDLK_SLASH, GAMEPLAY_EVENTS::REN_DBG_1}
    };

    m_gameplayBindings.m_gamepadBindings = {
        {SDL_CONTROLLER_BUTTON_A, GAMEPLAY_EVENTS::UP},
        {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, GAMEPLAY_EVENTS::RIGHT},
        {SDL_CONTROLLER_BUTTON_DPAD_DOWN, GAMEPLAY_EVENTS::DOWN},
        {SDL_CONTROLLER_BUTTON_DPAD_LEFT, GAMEPLAY_EVENTS::LEFT},
        {SDL_CONTROLLER_BUTTON_X, GAMEPLAY_EVENTS::ATTACK},
        {SDL_CONTROLLER_BUTTON_RIGHTSTICK, GAMEPLAY_EVENTS::FN1},
        {SDL_CONTROLLER_BUTTON_START, GAMEPLAY_EVENTS::FN2},
        {SDL_CONTROLLER_BUTTON_BACK, GAMEPLAY_EVENTS::FN3},
        {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, GAMEPLAY_EVENTS::CAM_STOP},
        {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, GAMEPLAY_EVENTS::FN4},
        {SDL_CONTROLLER_BUTTON_LEFTSTICK, GAMEPLAY_EVENTS::REN_DBG_1}
    };

    m_gameplayBindings.m_gamepadPositiveAxisBindings = {
        {SDL_CONTROLLER_AXIS_LEFTX, GAMEPLAY_EVENTS::RIGHT},
        {SDL_CONTROLLER_AXIS_LEFTY, GAMEPLAY_EVENTS::DOWN}
    };

    m_gameplayBindings.m_gamepadNegativeAxisBindings = {
        {SDL_CONTROLLER_AXIS_LEFTX, GAMEPLAY_EVENTS::LEFT},
        {SDL_CONTROLLER_AXIS_LEFTY, GAMEPLAY_EVENTS::UP}
    };

    m_hudBindings.m_keyboardBindings = {
        {SDLK_w, HUD_EVENTS::UP},
        {SDLK_d, HUD_EVENTS::RIGHT},
        {SDLK_s, HUD_EVENTS::DOWN},
        {SDLK_a, HUD_EVENTS::LEFT},
        {SDLK_i, HUD_EVENTS::PROCEED},
    };

    m_hudBindings.m_gamepadBindings = {
        {SDL_CONTROLLER_BUTTON_DPAD_UP, HUD_EVENTS::UP},
        {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, HUD_EVENTS::RIGHT},
        {SDL_CONTROLLER_BUTTON_DPAD_DOWN, HUD_EVENTS::DOWN},
        {SDL_CONTROLLER_BUTTON_DPAD_LEFT, HUD_EVENTS::LEFT},
        {SDL_CONTROLLER_BUTTON_A, HUD_EVENTS::PROCEED},
    };

    m_hudBindings.m_gamepadPositiveAxisBindings = {
        {SDL_CONTROLLER_AXIS_LEFTX, HUD_EVENTS::RIGHT},
        {SDL_CONTROLLER_AXIS_LEFTY, HUD_EVENTS::DOWN}
    };

    m_hudBindings.m_gamepadNegativeAxisBindings = {
        {SDL_CONTROLLER_AXIS_LEFTX, HUD_EVENTS::LEFT},
        {SDL_CONTROLLER_AXIS_LEFTY, HUD_EVENTS::UP}
    };
}

template <typename InputT, typename ButtonT, typename EventT>
inline void InputSystem::resolveBinding(const std::map<InputT, EventT> &bindings_, const ButtonT &input_, float value_)
{
    auto res = bindings_.find(InputT(input_));
    if (res != bindings_.end())
    {
        send(res->second, value_);
    }
}

InputReactor::InputReactor(InputSystem &input_) :
    m_input(input_)
{
}

void InputReactor::receiveEvents(GAMEPLAY_EVENTS event, const float scale_)
{
}

void InputReactor::receiveEvents(HUD_EVENTS event, const float scale_)
{
}

void InputReactor::setInputEnabled()
{
    m_inputEnabled = true;
}

void InputReactor::setInputDisabled()
{
    m_inputEnabled = false;
}

bool InputReactor::isInputEnabled()
{
    return m_inputEnabled;
}

InputReactor::~InputReactor()
{
    unsubscribeFromAll();
}

void InputReactor::subscribe(GAMEPLAY_EVENTS ev_)
{
    if (m_gameplaySubscriptions.contains(ev_))
        return;
    
    m_input.subscribe(ev_, this);
    m_gameplaySubscriptions.insert(ev_);
}

void InputReactor::subscribe(HUD_EVENTS ev_)
{
    if (m_hudSubscriptions.contains(ev_))
        return;
    
    m_input.subscribe(ev_, this);
    m_hudSubscriptions.insert(ev_);
}

void InputReactor::unsubscribe(GAMEPLAY_EVENTS ev_)
{
    if (!m_gameplaySubscriptions.contains(ev_))
        return;
    
    m_input.unsubscribe(ev_, this);
    m_gameplaySubscriptions.erase(ev_);
}

void InputReactor::unsubscribe(HUD_EVENTS ev_)
{
    if (!m_hudSubscriptions.contains(ev_))
        return;
    
    m_input.unsubscribe(ev_, this);
    m_hudSubscriptions.erase(ev_);
}

void InputReactor::unsubscribeFromAll()
{
    while(!m_gameplaySubscriptions.empty())
    {
        unsubscribe(*m_gameplaySubscriptions.begin());
    }

    while(!m_hudSubscriptions.empty())
    {
        unsubscribe(*m_hudSubscriptions.begin());
    }
}

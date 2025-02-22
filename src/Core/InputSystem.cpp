#include "InputSystem.h"
#include <iostream>

namespace 
{
    const std::map<Uint8, std::string> gamepadButtonNames = {
        {SDL_CONTROLLER_BUTTON_INVALID, "INVALID"},
        {SDL_CONTROLLER_BUTTON_A, "A"},
        {SDL_CONTROLLER_BUTTON_B, "B"},
        {SDL_CONTROLLER_BUTTON_X, "X"},
        {SDL_CONTROLLER_BUTTON_Y, "Y"},
        {SDL_CONTROLLER_BUTTON_BACK, "BACK"},
        {SDL_CONTROLLER_BUTTON_GUIDE, "GUIDE"},
        {SDL_CONTROLLER_BUTTON_START, "START"},
        {SDL_CONTROLLER_BUTTON_LEFTSTICK, "LEFTSTICK"},
        {SDL_CONTROLLER_BUTTON_RIGHTSTICK, "RIGHTSTICK"},
        {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "LEFTSHOULDER"},
        {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, "RIGHTSHOULDER"},
        {SDL_CONTROLLER_BUTTON_DPAD_UP, "DPAD_UP"},
        {SDL_CONTROLLER_BUTTON_DPAD_DOWN, "DPAD_DOWN"},
        {SDL_CONTROLLER_BUTTON_DPAD_LEFT, "DPAD_LEFT"},
        {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, "DPAD_RIGHT"},
        {SDL_CONTROLLER_BUTTON_MISC1, "MISC1"},
        {SDL_CONTROLLER_BUTTON_PADDLE1, "PADDLE1"},
        {SDL_CONTROLLER_BUTTON_PADDLE2, "PADDLE2"},
        {SDL_CONTROLLER_BUTTON_PADDLE3, "PADDLE3"},
        {SDL_CONTROLLER_BUTTON_PADDLE4, "PADDLE4"},
        {SDL_CONTROLLER_BUTTON_TOUCHPAD, "TOUCHPAD"},
        {SDL_CONTROLLER_BUTTON_MAX, "MAX"}
    };

    const std::map<Uint8, std::string> gamepadAxisNames = {
        {SDL_CONTROLLER_AXIS_INVALID, "INVALID"},
        {SDL_CONTROLLER_AXIS_LEFTX, "LEFTX"},
        {SDL_CONTROLLER_AXIS_LEFTY, "LEFTY"},
        {SDL_CONTROLLER_AXIS_RIGHTX, "RIGHTX"},
        {SDL_CONTROLLER_AXIS_RIGHTY, "RIGHTY"},
        {SDL_CONTROLLER_AXIS_TRIGGERLEFT, "TRIGGERLEFT"},
        {SDL_CONTROLLER_AXIS_TRIGGERRIGHT, "TRIGGERRIGHT"},
        {SDL_CONTROLLER_AXIS_MAX, "MAX"}
    };
}

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
            auto res = m_keyboardBindings.find(e.key.keysym.sym);
            if (res != m_keyboardBindings.end())
            {
                send(res->second, 1);
            }
            
        }
            break;

        case (SDL_KEYUP):
        {
            auto res = m_keyboardBindings.find(e.key.keysym.sym);
            if (res != m_keyboardBindings.end())
            {
                send(res->second, -1);
            }
        }
        break;

        case (SDL_CONTROLLERDEVICEADDED):
            m_controllers[e.cdevice.which].m_controller = SDL_GameControllerOpen(e.cdevice.which);
            std::cout << "Discovered new controller at " << e.cdevice.which << ": " << SDL_GameControllerName(m_controllers[e.cdevice.which].m_controller) << std::endl << SDL_GameControllerMapping(m_controllers[e.cdevice.which].m_controller) << std::endl;
        break;

        case (SDL_CONTROLLERDEVICEREMOVED):
            std::cout << "Removing controller at " << e.cdevice.which << std::endl;
            SDL_GameControllerClose(m_controllers[e.cdevice.which].m_controller);
            m_controllers.erase(e.cdevice.which);
        break;

        case (SDL_CONTROLLERBUTTONDOWN):
        {
            //std::cout << "Controller pressed " << gamepadButtonNames.at(e.cbutton.button) << std::endl;

            auto res = m_gamepadBindings.find(e.cbutton.button);
            if (res != m_gamepadBindings.end())
            {
                send(res->second, 1);
            }
        }
        break;

        case (SDL_CONTROLLERBUTTONUP):
        {
            //std::cout << "Controller released " << gamepadButtonNames.at(e.cbutton.button) << std::endl;

            auto res = m_gamepadBindings.find(e.cbutton.button);
            if (res != m_gamepadBindings.end())
            {
                send(res->second, -1);
            }
        }
        break;

        case (SDL_CONTROLLERAXISMOTION):
        {
            float posValue, negValue;
            if (e.caxis.value > 0)
            {
                if (e.caxis.value < m_stickDeadzone)
                    posValue = 0;
                else
                    posValue = e.caxis.value / 32767.0f;
                negValue = 0;
            }
            else
            {
                if (-e.caxis.value < m_stickDeadzone)
                    negValue = 0;
                else
                    negValue = e.caxis.value / -32768.0f;
                posValue = 0;
            }

            auto resPos = m_gamepadPositiveAxisBindings.find(e.caxis.axis);
            if (resPos != m_gamepadPositiveAxisBindings.end())
                send(resPos->second, posValue);

            auto resNeg = m_gamepadNegativeAxisBindings.find(e.caxis.axis);
            if (resNeg != m_gamepadNegativeAxisBindings.end())
                send(resNeg->second, negValue);

            std::cout << "Controller axis motion " << gamepadAxisNames.at(e.caxis.axis) << " : " << int(e.caxis.value) << std::endl;
        }
        break;

        default:
            //std::cout << "Unknown event " << int(e.type) << std::endl;
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

void InputSystem::initiateControllers()
{
    std::cout << "Discovering controllers..." << std::endl;
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            auto &ctrl = m_controllers[i];
            ctrl.m_controller = SDL_GameControllerOpen(i);
            std::cout << "Found supported controller at " << i << ": " << SDL_GameControllerName(ctrl.m_controller) << std::endl << SDL_GameControllerMapping(ctrl.m_controller) << std::endl;
        }
        else
        {
            std::cout << "Found unknown joystick at " << i << std::endl;
        }
    }
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

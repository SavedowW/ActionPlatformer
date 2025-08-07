#include "InputSystem.h"
#include "JsonUtils.hpp"
#include "FilesystemUtils.h"
#include <iostream>
#include <filesystem>
#include <fstream>

InputSystem::InputSystem() :
    m_rootPath(Filesystem::getRootDirectory() + "Configs"),
    m_configPath(Filesystem::getRootDirectory() + "Configs/inputs.json")

{
    if (!std::filesystem::exists(m_configPath))
    {
        std::cout << m_configPath << " was not found, initializing inputs config" << std::endl;
        setupDefaultMapping();
        exportMappingAs(m_configPath);
    }
    else
    {
        std::cout << "Loading config from " << m_configPath << std::endl;
        importMappingEnsureUnique(m_configPath);
    }
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
void InputSystem::subscribe(GAMEPLAY_EVENTS ev_, Subscriber sub_)
{
    assert(sub_);

    m_gameplaySubscribers[(int)ev_].push_back(sub_);
}

//Automatically called when InputReactor is destroyed
void InputSystem::unsubscribe(GAMEPLAY_EVENTS ev_, Subscriber sub_)
{
    std::vector<Subscriber>& v = m_gameplaySubscribers[(int)ev_];
}

void InputSystem::subscribe(HUD_EVENTS ev_, Subscriber sub_)
{
    assert(sub_);

    m_hudSubscribers[(int)ev_].push_back(sub_);
}

void InputSystem::unsubscribe(HUD_EVENTS ev_, Subscriber sub_)
{
    std::vector<Subscriber>& v = m_hudSubscribers[(int)ev_];
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
    for (auto& sub : m_gameplaySubscribers[(int)ev_])
    {
        if (sub->isInputEnabled())
        {
            sub->receiveEvents(ev_, val_);
        }
    }
}

void InputSystem::send(HUD_EVENTS ev_, float val_)
{
    for (auto& sub : m_hudSubscribers[(int)ev_])
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
        {SDLK_SLASH, GAMEPLAY_EVENTS::REN_DBG_1},
        {SDLK_r, GAMEPLAY_EVENTS::RESET_DBG}
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
        {SDL_CONTROLLER_BUTTON_LEFTSTICK, GAMEPLAY_EVENTS::REN_DBG_1},
        {SDL_CONTROLLER_BUTTON_Y, GAMEPLAY_EVENTS::RESET_DBG}
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

void InputSystem::exportMappingAs(const std::string &fileName_)
{
    nlohmann::json out;

    utils::exportMap(out["GameplayKeyboard"], m_gameplayBindings.m_keyboardBindings);
    utils::exportMap(out["GameplayGamepad"], m_gameplayBindings.m_gamepadBindings);
    utils::exportMap(out["GameplayAxisPos"], m_gameplayBindings.m_gamepadPositiveAxisBindings);
    utils::exportMap(out["GameplayAxisNeg"], m_gameplayBindings.m_gamepadNegativeAxisBindings);

    utils::exportMap(out["HudKeyboard"], m_hudBindings.m_keyboardBindings);
    utils::exportMap(out["HudGamepad"], m_hudBindings.m_gamepadBindings);
    utils::exportMap(out["HudAxisPos"], m_hudBindings.m_gamepadPositiveAxisBindings);
    utils::exportMap(out["HudAxisNeg"], m_hudBindings.m_gamepadNegativeAxisBindings);

    std::ofstream fout(fileName_);
    fout << out.dump(4);
}

void InputSystem::importMappingEnsureUnique(const std::string &fileName_)
{
    bool result = false;

    std::ifstream injson(fileName_);
    if (!injson.is_open())
    {
        std::cout << "Failed to open mapping at \"" << fileName_ << "\"\n";
        return;
    }

    nlohmann::json in = nlohmann::json::parse(injson);
    injson.close();

    m_gameplayBindings.m_keyboardBindings.clear();
    m_gameplayBindings.m_gamepadBindings.clear();
    m_gameplayBindings.m_gamepadPositiveAxisBindings.clear();
    m_gameplayBindings.m_gamepadNegativeAxisBindings.clear();
    m_hudBindings.m_keyboardBindings.clear();
    m_hudBindings.m_gamepadBindings.clear();
    m_hudBindings.m_gamepadPositiveAxisBindings.clear();
    m_hudBindings.m_gamepadNegativeAxisBindings.clear();

    result = utils::importMapEnsureUnique(in["GameplayKeyboard"], m_gameplayBindings.m_keyboardBindings, result);
    result = utils::importMapEnsureUnique(in["GameplayGamepad"], m_gameplayBindings.m_gamepadBindings, result);
    result = utils::importMapEnsureUnique(in["GameplayAxisPos"], m_gameplayBindings.m_gamepadPositiveAxisBindings, result);
    result = utils::importMapEnsureUnique(in["GameplayAxisNeg"], m_gameplayBindings.m_gamepadNegativeAxisBindings, result);

    result = utils::importMapEnsureUnique(in["HudKeyboard"], m_hudBindings.m_keyboardBindings, result);
    result = utils::importMapEnsureUnique(in["HudGamepad"], m_hudBindings.m_gamepadBindings, result);
    result = utils::importMapEnsureUnique(in["HudAxisPos"], m_hudBindings.m_gamepadPositiveAxisBindings, result);
    result = utils::importMapEnsureUnique(in["HudAxisNeg"], m_hudBindings.m_gamepadNegativeAxisBindings, result);

    if (result)
    {
        std::cout << fileName_ << " included duplicated values, resaving file" << std::endl;
        exportMappingAs(fileName_);
    }
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
    while (!m_gameplaySubscriptions.empty())
    {
        unsubscribe(*m_gameplaySubscriptions.begin());
    }

    while (!m_hudSubscriptions.empty())
    {
        unsubscribe(*m_hudSubscriptions.begin());
    }
}

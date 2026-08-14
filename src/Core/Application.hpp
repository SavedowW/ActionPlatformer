#include "Application.h"
#include <concepts>
#include <stdexcept>

template<typename T, typename... Args>
void Application::makeLevel(Args&&... args_)
    requires std::constructible_from<T, FPSUtility&, Args...>
{
    auto level = std::make_unique<T>(m_fpsUtility, std::forward<Args>(args_)...);
    const auto lvlname = level->name();

    const auto res = m_levels.insert({lvlname, std::move(level)});

    if (!res.second)
        throw std::runtime_error(std::format("Level \"{}\" already exists", lvlname));

    if (!m_levelResult.nextLvl.has_value())
        m_levelResult.nextLvl = lvlname;
    
}

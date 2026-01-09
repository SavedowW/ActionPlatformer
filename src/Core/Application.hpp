#include "Application.h"
#include <concepts>

template<typename T, typename... Args>
void Application::makeLevel(int levelId_, Args&&... args_)
    requires std::constructible_from<T, int, FPSUtility&, Args...>
{
    if (m_levels.contains(levelId_))
    {
        std::cout << "Warning: a level with ID " << levelId_ << " already exists" << std::endl;
        return;
    }

    m_levels.insert({levelId_, std::make_unique<T>(levelId_, m_fpsUtility, std::forward<Args>(args_)...)});
}

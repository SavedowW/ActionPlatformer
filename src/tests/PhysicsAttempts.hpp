#include "PhysicsSystem.h"
#include "Core/Logger.hpp"

std::string stringifyAttempt(const AttemptPos &attempt_)
{
    return std::format("{}, {}, {}", attempt_.pos, (attempt_.requireMagnet ? "magnet" : "nomagn"), (attempt_.haltMovement ? "haltmv" : "nohalt"));
}

void testPhysicsAttempts()
{
    try
    {
        LOG_TRACE("Moving to the right");
        AttemptContainerRight container;

        container.add(AttemptPos{{10, 0}, true, false, AttemptType::BACKWARD});
        container.add(AttemptPos{{10, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{11, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{8, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{12, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{7, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        
        LOG_TRACE(stringifyAttempt(container.extract()));
        LOG_TRACE(stringifyAttempt(container.extract()));

        container.add(AttemptPos{{12, -1}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{12, -1}, true, true, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{10, -3}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{8, 1}, true, true, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{10, -3}, true, true, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{10, -4}, true, true, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{10, -4}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{8, 1}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));

        while (!container.empty())
            LOG_TRACE(stringifyAttempt(container.extract()));

        LOG_TRACE("Next one should be exception");
        LOG_TRACE(stringifyAttempt(container.extract()));
    }
    catch (const std::exception &ex_)
    {
        LOG_ERROR(ex_.what());
    }

    try
    {
        LOG_TRACE("Moving to the left");
        AttemptContainerLeft container;

        container.add(AttemptPos{{10, 0}, true, false, AttemptType::BACKWARD});
        container.add(AttemptPos{{10, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{11, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{8, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{12, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{7, 0}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        
        LOG_TRACE(stringifyAttempt(container.extract()));
        LOG_TRACE(stringifyAttempt(container.extract()));

        container.add(AttemptPos{{12, -1}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{12, -1}, true, true, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{10, -3}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{8, 1}, true, true, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{10, -3}, true, true, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{10, -4}, true, true, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{10, -4}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));
        container.add(AttemptPos{{8, 1}, true, false, AttemptType::BACKWARD}.addIgnoredObstacle(entt::null));

        while (!container.empty())
            LOG_TRACE(stringifyAttempt(container.extract()));

        LOG_TRACE("Next one should be exception");
        LOG_TRACE(stringifyAttempt(container.extract()));
    }
    catch (const std::exception &ex_)
    {
        LOG_ERROR(ex_.what());
    }
}

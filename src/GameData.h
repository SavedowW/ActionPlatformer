#ifndef GAMEDATA_H_
#define GAMEDATA_H_
#include <string_view>
#include <SDL.h>
#include "Vector2.h"
#include <array>

namespace gamedata
{
    namespace global
    {
        inline constexpr float framerate = 60.0f;
        inline constexpr int numberOfLevels = 2;
        inline constexpr int initialLevelId = 1;
        inline constexpr Vector2<float> baseResolution = {1280.0f, 720.0f};
        inline constexpr Vector2<float> maxCameraSize = {416.0f, 234.0f};
        inline constexpr Vector2<float> minCameraSize = {288.0f, 162.0f};
        inline constexpr float maxCameraScale = maxCameraSize.y / baseResolution.y;
        inline constexpr float minCameraScale = minCameraSize.y / baseResolution.y;
        inline constexpr int inputBufferLength = 4;
    }

    namespace characters
    {
        inline constexpr SDL_Color pushboxColor = {255, 242, 0, 100};
        inline constexpr SDL_Color hitboxColor = {255, 0, 0, 100};
        inline constexpr SDL_Color hurtboxColor = {0, 255, 0, 100};
    }
}

#endif
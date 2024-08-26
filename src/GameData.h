#ifndef GAMEDATA_H_
#define GAMEDATA_H_
#include <string_view>
#include <SDL.h>
#include "Vector2.h"
#include <array>

// TODO: move properties to config file

namespace gamedata
{
    namespace colors
    {
        inline constexpr SDL_Color LVL1 = {233, 239, 236, 255};
        inline constexpr SDL_Color LVL2 = {160, 160, 139, 255};
        inline constexpr SDL_Color LVL3 = {85, 85, 104, 255};
        inline constexpr SDL_Color LVL4 = {33, 30, 32, 255};
    }

    namespace debug
    {
        inline constexpr bool drawColliders = true;
        inline constexpr bool drawFocusAreas = true;
        inline constexpr bool drawCameraOffset = true;
        inline constexpr bool drawNpsDebug = true;
        inline constexpr bool drawCurrentConnection = true;
        inline constexpr bool drawNavGraph = true;
        inline constexpr bool drawTransforms = false;
        inline constexpr bool drawDebugTextures = false;
        inline constexpr uint32_t debugPathDisplay = 15;

        inline constexpr bool dumpSystemDuration = true;
    }

    namespace global
    {
        inline constexpr float framerate = 60.0f;
        inline constexpr float dbgslowdownfps = 20.0f;
        inline constexpr Vector2<float> tileSize = {16.0f, 16.0f};
        inline constexpr int numberOfLevels = 2;
        inline constexpr int initialLevelId = 1;
        inline constexpr Vector2<int> defaultWindowResolution = {1280, 720};
        inline constexpr Vector2<float> baseResolution = {320.0f, 180.0f}; // 320 x 180 or 480 x 270
        inline constexpr Vector2<float> maxCameraSize = {640.0f, 360.0f};
        inline constexpr Vector2<float> minCameraSize = {320.0f, 180.0f};
        inline constexpr float baseCameraScale = 1.0f;
        inline constexpr float maxCameraScale = maxCameraSize.y / baseResolution.y;
        inline constexpr float minCameraScale = minCameraSize.y / baseResolution.y;
        inline constexpr int inputBufferLength = 4;
        inline constexpr size_t renderLayerCount = 3;
    }

    namespace characters
    {
        inline constexpr SDL_Color pushboxColor = {255, 242, 0, 100};
        inline constexpr SDL_Color hitboxColor = {255, 0, 0, 100};
        inline constexpr SDL_Color hurtboxColor = {0, 255, 0, 100};
    }
}

#define EXPECTED_RENDER_LAYERS(cnt) if constexpr (gamedata::global::renderLayerCount != cnt) throw std::string("Unexpected layer count: ") + std::to_string(cnt) + " vs " + std::to_string(gamedata::global::renderLayerCount);

#endif
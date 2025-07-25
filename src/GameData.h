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
        inline constexpr bool drawFocusAreas = false;
        inline constexpr bool drawCameraOffset = false;
        inline constexpr bool drawNpcDebug = false;
        inline constexpr bool drawCurrentConnection = false;
        inline constexpr bool drawNavGraph = false;
        inline constexpr bool drawTransforms = true;
        inline constexpr bool drawDebugTextures = false;
        inline constexpr bool drawHealthPos = false;
        inline constexpr bool drawColliderRoutes = true;
        inline constexpr uint32_t debugPathDisplay = 0;
    }

    namespace global
    {
        inline constexpr float framerate = 60.0f;
        inline constexpr float dbgslowdownfps = 20.0f;
        inline constexpr Vector2<int> baseResolution = {640, 360}; // 320 x 180 or 480 x 270
        inline constexpr Vector2<int> maxCameraSize = {640, 360};
        inline constexpr Vector2<int> minCameraSize = {320, 180};
        inline constexpr Vector2<int> hudLayerResolution = {640, 360};
        inline constexpr float baseCameraScale = maxCameraSize.y / (float)baseResolution.y;
        inline constexpr float minCameraScale = (float)maxCameraSize.y / maxCameraSize.y;
        inline constexpr float maxCameraScale = (float)maxCameraSize.y / minCameraSize.y;
        inline constexpr int inputBufferLength = 4;
    }

    namespace tiles
    {
        inline constexpr Vector2<float> tileSize = {16.0f, 16.0f};
    }

    namespace characters
    {
        inline constexpr SDL_Color pushboxColor = {255, 242, 0, 50};
        inline constexpr SDL_Color hitboxColor = {255, 0, 0, 100};
        inline constexpr SDL_Color hurtboxColor = {0, 255, 0, 100};
    }
}

#endif

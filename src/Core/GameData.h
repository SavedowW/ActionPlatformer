#pragma once
#include "Color.hpp"
#include "Vector2.hpp"

// TODO: move properties to config file
// split remaining ones and defaults into different files

namespace gamedata
{
    namespace colors
    {
        inline constexpr Color LVL1{233, 239, 236, 255};
        inline constexpr Color LVL2{160, 160, 139, 255};
        inline constexpr Color LVL3{85, 85, 104, 255};
        inline constexpr Color LVL4{33, 30, 32, 255};
    }

    namespace debug_defaults
    {
        inline constexpr bool drawColliders = false;
        inline constexpr bool drawFocusAreas = false;
        inline constexpr bool drawCameraOffset = false;
        inline constexpr bool drawNpcDebug = false;
        inline constexpr bool drawCurrentConnection = false;
        inline constexpr bool drawNavGraph = false;
        inline constexpr bool drawTransforms = false;
        inline constexpr bool drawDebugTextures = false;
        inline constexpr bool drawHealthPos = false;
        inline constexpr bool drawColliderRoutes = false;
        inline constexpr uint32_t debugPathDisplay = 0;
    }

    namespace global
    {
        inline constexpr Vector2<uint16_t> baseResolution = {640, 360}; // 320 x 180 or 480 x 270
        inline constexpr Vector2<uint16_t> maxCameraSize = {640, 360};
        inline constexpr Vector2<uint16_t> minCameraSize = {320, 180};
        inline constexpr Vector2<uint16_t> hudLayerResolution = {640, 360};
        inline constexpr float baseCameraScale = maxCameraSize.y / (float)baseResolution.y;
        inline constexpr float minCameraScale = (float)maxCameraSize.y / maxCameraSize.y;
        inline constexpr float maxCameraScale = (float)maxCameraSize.y / minCameraSize.y;
        inline constexpr unsigned int inputBufferLength = 4;
    }

    namespace tiles
    {
        inline constexpr Vector2<int> tileSize = {16, 16};
    }

    namespace characters
    {
        inline constexpr Color pushboxColor{115, 94, 74, 100};
        inline constexpr Color hitboxColor{255, 0, 0, 100};
        inline constexpr Color hurtboxColor{0, 255, 0, 100};
    }
}

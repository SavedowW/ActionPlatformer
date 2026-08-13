#pragma once
#include "Core/SlidingWindow.h"
#include "Core/Texture.h"
#include "Core/CoreComponents.h"
#include "Core/Camera.h"
#include "PlayerSystem.h"
#include <entt/entt.hpp>

struct HudSystem
{
public:
    HudSystem(entt::registry &reg_, Camera &cam_, int lvlId_, const Vector2<float> &lvlSize_, const PlayerSystem &playersys_);

    void draw() const;
    void drawCommonDebug() const;
    void drawPlayerDebug(entt::entity playerId_) const;
    void drawNPCDebug(const ComponentTransform &trans_, const ComponentPhysical &phys_) const;

    void setPlayerId(entt::entity playerId_) noexcept;

    
private:
    Renderer &m_renderer;
    const Window &m_window;
    TextManager &m_textManager;
    entt::registry &m_reg;
    const PlayerSystem &m_playersys;

    Camera &m_cam;
    int m_lvlId;
    const Vector2<float> m_lvlSize;
    mutable SlidingWindowIterative<float, 20, 5> m_avgFrames;

    std::shared_ptr<Texture> m_arrowIn;
    std::shared_ptr<Texture> m_arrowOut;
};

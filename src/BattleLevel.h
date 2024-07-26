#ifndef BATTLE_LEVEL_H_
#define BATTLE_LEVEL_H_

#include "CameraFocusArea.h"
#include "Level.h"
#include "Camera.h"
#include "Application.h"
#include <array>
#include "FrameTimer.h"
#include <SDL_ttf.h>
#include <ranges>
#include <set>
#include "DebugDataWidget.h"
#include "CollisionArea.h"
#include "DecorationBuilder.h"
#include "yaECS.hpp"

//using ArchPlayer = ECS::EntityData<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, ComponentAnimationRenderable, ComponentPlayerInput>;
//using ArchMob = ECS::EntityData<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, ComponentAnimationRenderable>;

// Make an archetype list for example
//using MyReg = ECS::ArchList<>
//    ::addTypelist<ArchPlayer::WithSM<CharacterState>>;

class BattleLevel : public Level
{
public:
    BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_);
    virtual void enter() override;

    virtual ~BattleLevel() = default;

protected:
    void update() override;
    void draw() override;

    bool updateFocus();

    HUD m_hud;
    Camera m_camera;

    CollisionArea m_collisionArea;

    std::vector<CameraFocusArea> m_camFocusAreas;
    CameraFocusArea *m_currentCamFocusArea = nullptr;

    DecorLayers m_decor;
    Tileset m_tlmap;

    //ECS::Registry<MyReg> m_registry;
};

#endif

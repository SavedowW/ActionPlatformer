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
#include "PlayableCharacter.h"
#include "yaECS.hpp"

using ArchPlayer = ECS::EntityData<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, ComponentAnimationRenderable, ComponentPlayerInput>;
//using ArchMob = ECS::EntityData<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, ComponentAnimationRenderable>;

// Make an archetype list for example
using MyReg = ECS::ArchList<>
    ::addTypelist<ArchPlayer::WithSM<CharacterState>>;

struct PlayerSystem
{
    PlayerSystem(ECS::Registry<MyReg> &reg_, Application &app_);

    void setup();

    using PlayerQuery = std::invoke_result_t<decltype(&ECS::Registry<MyReg>::getQueryTl<
    ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, ComponentAnimationRenderable, ComponentPlayerInput, 
    StateMachine<ArchPlayer::MakeRef, CharacterState>>), 
    ECS::Registry<MyReg>, ArchPlayer::WithSM<CharacterState>>;

    PlayerQuery m_query;
    AnimationManager &m_animManager;
};

struct RenderSystem
{
    RenderSystem(ECS::Registry<MyReg> &reg_, Application &app_, Camera &camera_);

    void draw();

    template<typename T>
    void drawArch(T &arch_)
    {
        for (int i = 0; i < arch_.size(); ++i)
        {
            auto inst = arch_.getEntity(i);

            drawInstance(std::get<ComponentTransform&>(inst), std::get<ComponentAnimationRenderable&>(inst));

            if constexpr (T::template containsOne<ComponentPhysical>())
            {
                drawCollider(std::get<ComponentTransform&>(inst), std::get<ComponentPhysical&>(inst));
            }
        }
    }

    void drawInstance(ComponentTransform &trans_, ComponentAnimationRenderable &ren_);
    void drawCollider(ComponentTransform &trans_, ComponentPhysical &phys_);

    using PlayerQuery = std::invoke_result_t<decltype(&ECS::Registry<MyReg>::getQuery<ComponentAnimationRenderable>), 
    ECS::Registry<MyReg>>;

    PlayerQuery m_query;
    Renderer &m_renderer;
    Camera &m_camera;
};

struct InputHandlingSystem
{
    InputHandlingSystem(ECS::Registry<MyReg> &reg_);

    void update();

    template<typename T>
    void updateArch(T &arch_)
    {
        auto &inps = arch_.get<ComponentPlayerInput>();
        for (int i = 0; i < arch_.size(); ++i)
        {
            inps[i].m_inputResolver->update();
        }
    }

    using InputQuery = std::invoke_result_t<decltype(&ECS::Registry<MyReg>::getQuery<ComponentPlayerInput>), 
    ECS::Registry<MyReg>>;
    InputQuery m_query;
};

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

    ECS::Registry<MyReg> m_registry;
    PlayerSystem m_playerSystem;
    RenderSystem m_rendersys;
    InputHandlingSystem m_inputsys;
};

#endif

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
#include "DecorationBuilder.h"
#include "PlayableCharacter.h"
#include "yaECS.hpp"

//using ArchMob = ECS::EntityData<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, ComponentAnimationRenderable>;

// Make an archetype list for example
using MyReg = ECS::ArchList<>
    ::addTypelist<ArchPlayer::WithSM<CharacterState>>
    ::add<ComponentStaticCollider> // Expected to be unchanged (no entities removed, added or switched positions)
    ::add<ComponentStaticCollider, ComponentObstacle> // Expected to be unchanged (no entities removed, added or switched positions)
    ::add<ComponentTrigger>; // Expected to be unchanged (no entities removed, added or switched positions)

/* 
    TODO: porting:
    Camera system (track player, handle focus areas)
    Camera focus areas
    Rest of the states
    Triggers, wall clinging
    Custom float state (speed up after forcing falling, air drift)
*/

struct PlayerSystem
{
    PlayerSystem(ECS::Registry<MyReg> &reg_, Application &app_);

    void setup();

    void update();

    template<typename T>
    void updateArch(T &arch_)
    {
        auto &smcs = arch_.template get<StateMachine<ArchPlayer::MakeRef, CharacterState>>();
        for (int i = 0; i < arch_.size(); ++i)
        {
            auto inst = arch_.template getEntity<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough, ComponentAnimationRenderable, ComponentPlayerInput>(i);
            auto &smc = smcs[i];
            smc.update(inst, 0);
        }
    }

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

            if constexpr (T::template containsOne<ComponentStaticCollider>() && !T::template containsOne<ComponentObstacle>())
            {
                drawCollider(std::get<ComponentStaticCollider&>(inst));
            }

            if constexpr (T::template containsOne<ComponentStaticCollider>() && T::template containsOne<ComponentObstacle>())
            {
                drawObstacle(std::get<ComponentStaticCollider&>(inst));
            }

            if constexpr (T::template containsOne<ComponentTrigger>())
            {
                drawTrigger(std::get<ComponentTrigger&>(inst));
            }

            if constexpr (T::template containsOne<ComponentTransform>() && T::template containsOne<ComponentAnimationRenderable>())
            {
                drawInstance(std::get<ComponentTransform&>(inst), std::get<ComponentAnimationRenderable&>(inst));
            }

            if constexpr (T::template containsOne<ComponentPhysical>())
            {
                drawCollider(std::get<ComponentTransform&>(inst), std::get<ComponentPhysical&>(inst));
            }
        }
    }

    void drawInstance(ComponentTransform &trans_, ComponentAnimationRenderable &ren_);
    void drawCollider(ComponentTransform &trans_, ComponentPhysical &phys_);
    void drawCollider(ComponentStaticCollider &cld_);
    void drawObstacle(ComponentStaticCollider &cld_);
    void drawTrigger(ComponentTrigger &cld_);

    using PlayerQuery = std::invoke_result_t<decltype(&ECS::Registry<MyReg>::getQuery<ComponentAnimationRenderable>), 
    ECS::Registry<MyReg>>;
    PlayerQuery m_query;

    using StaticColliderQuery = std::invoke_result_t<decltype(&ECS::Registry<MyReg>::getQuery<ComponentStaticCollider>), ECS::Registry<MyReg>>;
    StaticColliderQuery m_staticColliderQuery;

    using StaticTriggerQuery = std::invoke_result_t<decltype(&ECS::Registry<MyReg>::getQuery<ComponentTrigger>), ECS::Registry<MyReg>>;
    StaticTriggerQuery m_staticTriggerQuery;

    Renderer &m_renderer;
    Camera &m_camera;
};

struct PhysicsSystem
{
    PhysicsSystem(ECS::Registry<MyReg> &reg_, Vector2<float> levelSize_);

    void update();

    template<typename T>
    void updateArch(T &arch_)
    {
        for (int i = 0; i < arch_.size(); ++i)
        {
            auto inst = arch_.getEntity(i);
            if constexpr (T::template isLastSpecialization<StateMachine>())
                proceedEntity(std::get<ComponentTransform&>(inst), std::get<ComponentPhysical&>(inst), std::get<ComponentObstacleFallthrough&>(inst), &arch_.getLast()[i], TypeManip::tupleWithoutLast(inst));
            else
                proceedEntity(std::get<ComponentTransform&>(inst), std::get<ComponentPhysical&>(inst), std::get<ComponentObstacleFallthrough&>(inst), nullptr, inst);
        }
    }

    void proceedEntity(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, auto *sm_, auto inst_);
    
    bool magnetEntity(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    bool getHighestVerticalMagnetCoord(const Collider &cld_, float &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_) const;

    void resetEntityObstacles(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    std::set<int> getTouchedObstacles(const Collider &pb_) const;

    using PhysicalQuery = std::invoke_result_t<decltype(&ECS::Registry<MyReg>::getQuery<ComponentTransform, ComponentPhysical, ComponentObstacleFallthrough>), ECS::Registry<MyReg>>;
    PhysicalQuery m_physicalQuery;

    using StaticColliderQuery = std::invoke_result_t<decltype(&ECS::Registry<MyReg>::getQuery<ComponentStaticCollider>), ECS::Registry<MyReg>>;
    StaticColliderQuery m_staticColliderQuery;

    Vector2<float> m_levelSize;
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

    std::vector<CameraFocusArea> m_camFocusAreas;
    CameraFocusArea *m_currentCamFocusArea = nullptr;

    DecorLayers m_decor;
    Tileset m_tlmap;

    ECS::Registry<MyReg> m_registry;
    PlayerSystem m_playerSystem;
    RenderSystem m_rendersys;
    InputHandlingSystem m_inputsys;
    PhysicsSystem m_physsys;
};

#endif

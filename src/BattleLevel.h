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
    PlayerSystem(ECS::Registry<Components> &reg_, Application &app_);

    void setup();
    void update(ECS::Registry<Components> &reg_);

    ECS::Query<Components> m_query;
    AnimationManager &m_animManager;
};

struct RenderSystem
{
    RenderSystem(ECS::Registry<Components> &reg_, Application &app_, Camera &camera_);

    void draw();

    static void drawInstance(RenderSystem *rensys_, ComponentTransform &trans_, ComponentAnimationRenderable &ren_);
    static void drawCollider(RenderSystem *ren_, ComponentTransform &trans_, ComponentPhysical &phys_);
    static void drawCollider(RenderSystem *ren_, ComponentStaticCollider &cld_);
    static void drawObstacle(RenderSystem *ren_, ComponentStaticCollider &cld_);
    static void drawTrigger(RenderSystem *ren_, ComponentTrigger &cld_);

    ECS::Query<Components> m_query;
    ECS::Query<Components> m_staticColliderQuery;
    ECS::Query<Components> m_staticTriggerQuery;

    Renderer &m_renderer;
    Camera &m_camera;
};

struct PhysicsSystem
{
    PhysicsSystem(ECS::Registry<Components> &reg_, Vector2<float> levelSize_);

    void update();

    void proceedEntity(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, StateMachine *sm_, ECS::CheapEntityView<Components> &inst_);
    
    bool magnetEntity(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    bool getHighestVerticalMagnetCoord(const Collider &cld_, float &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_);

    void resetEntityObstacles(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    std::set<int> getTouchedObstacles(const Collider &pb_);

    ECS::Query<Components> m_physicalQuery;
    ECS::Query<Components> m_staticColliderQuery;

    Vector2<float> m_levelSize;
};


struct InputHandlingSystem
{
    InputHandlingSystem(ECS::Registry<Components> &reg_);

    void update();

    ECS::Query<Components> m_query;
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

    ECS::Registry<Components> m_registry;
    PlayerSystem m_playerSystem;
    RenderSystem m_rendersys;
    InputHandlingSystem m_inputsys;
    PhysicsSystem m_physsys;
};

#endif

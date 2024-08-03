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

/* 
    TODO: porting:
    Rest of the states
    Triggers, wall clinging
*/

struct PlayerSystem
{
    PlayerSystem(entt::registry &reg_, Application &app_);

    void setup();
    void update();

    entt::registry &m_reg;
    AnimationManager &m_animManager;
};

struct CameraSystem
{
    CameraSystem(entt::registry &reg_, Camera &cam_);

    void update();
    bool updateFocus(const Collider &playerPb_);

    entt::registry &m_reg;
    entt::entity m_playerId;
    Camera &m_cam;
    std::optional<entt::entity> m_currentFocusArea;
};

struct RenderSystem
{
    RenderSystem(entt::registry &reg_, Application &app_, Camera &camera_);

    void draw();

    void drawInstance(ComponentTransform &trans_, ComponentAnimationRenderable &ren_);
    void drawCollider(ComponentTransform &trans_, ComponentPhysical &phys_);
    void drawCollider(ComponentStaticCollider &cld_);
    void drawObstacle(ComponentStaticCollider &cld_);
    void drawTrigger(ComponentTrigger &cld_);
    void drawFocusArea(CameraFocusArea &cfa_);


    entt::registry &m_reg;
    Renderer &m_renderer;
    Camera &m_camera;
};

struct PhysicsSystem
{
    PhysicsSystem(entt::registry &reg_, Vector2<float> levelSize_);

    void update();

    void proceedEntity(auto &clds_, entt::entity idx_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_, StateMachine *sm_);
    
    bool magnetEntity(auto &clds_, ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    bool getHighestVerticalMagnetCoord(auto &clds_, const Collider &cld_, float &coord_, const std::set<int> ignoredObstacles_, bool ignoreAllObstacles_);

    void resetEntityObstacles(ComponentTransform &trans_, ComponentPhysical &phys_, ComponentObstacleFallthrough &obsFallthrough_);
    std::set<int> getTouchedObstacles(const Collider &pb_);

    entt::registry &m_reg;
    Vector2<float> m_levelSize;
};


struct InputHandlingSystem
{
    InputHandlingSystem(entt::registry &reg_);

    void update();

    entt::registry &m_reg;
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

    HUD m_hud;
    Camera m_camera;

    std::vector<CameraFocusArea> m_camFocusAreas;
    CameraFocusArea *m_currentCamFocusArea = nullptr;

    DecorLayers m_decor;
    Tileset m_tlmap;

    entt::registry m_registry;
    PlayerSystem m_playerSystem;
    RenderSystem m_rendersys;
    InputHandlingSystem m_inputsys;
    PhysicsSystem m_physsys;
    CameraSystem m_camsys;
};

#endif

#ifndef BATTLE_LEVEL_H_
#define BATTLE_LEVEL_H_

#include "Level.h"
#include "Camera.h"
#include "Application.h"
#include "ImmediateScreenLog.h"
#include "LevelBuilder.h"
#include "PlayableCharacter.h"
#include "PhysicsSystem.h"
#include "PlayerSystem.h"
#include "RenderSystem.h"
#include "CameraSystem.h"
#include "HUDSystem.h"
#include "InputHandlingSystem.h"
#include "EnemySystem.h"
#include "NavGraph.h"
#include "AISystem.h"
#include "NavSystem.h"
#include "DynamicColliderSystem.h"

class BattleLevel : public Level
{
public:
    BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_);
    virtual void enter() override;

    virtual ~BattleLevel() = default;

protected:
    void update() override;
    void draw() override;

    Camera m_camera;

    DecorLayers m_decor;
    Tileset m_tlmap;

    entt::registry m_registry;
    PlayerSystem m_playerSystem;
    RenderSystem m_rendersys;
    InputHandlingSystem m_inputsys;
    PhysicsSystem m_physsys;
    CameraSystem m_camsys;
    HudSystem m_hudsys;
    EnemySystem m_enemysys;
    AISystem m_aisys;
    NavSystem m_navsys;
    DynamicColliderSystem m_colsys;

    NavGraph m_graph;

    entt::entity m_playerId;
};

#endif

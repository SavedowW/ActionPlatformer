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
#include "ParticleSystem.h"
#include "BattleSystem.h"
#include "ChatBox.h"
#include "ColliderRouting.h"
#include "EnvironmentSystem.h"

class BattleLevel : public Level
{
public:
    BattleLevel(Application *application_, const Vector2<float>& size_, int lvlId_);
    virtual void enter() override;

    void receiveEvents(GAMEPLAY_EVENTS event, const float scale_) override;

    virtual ~BattleLevel() = default;

protected:
    void update() override;
    void draw() override;

    Camera m_camera;

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
    ParticleSystem m_partsys;
    BattleSystem m_battlesys;
    ChatboxSystem m_chatBoxSys;
    EnvironmentSystem m_envSystem;

    NavGraph m_graph;

    entt::entity m_playerId;
    entt::entity m_enemyId;

    ColliderRoutesCollection m_cldRoutesCollection;
    LevelBuilder m_lvlBuilder;
};

#endif

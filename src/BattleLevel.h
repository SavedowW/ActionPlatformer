#ifndef BATTLE_LEVEL_H_
#define BATTLE_LEVEL_H_

#include "LevelBuilder.h"
#include "PhysicsSystem.h"
#include "PlayerSystem.h"
#include "RenderSystem.h"
#include "CameraSystem.h"
#include "HUDSystem.h"
#include "InputHandlingSystem.h"
#include "EnemySystem.h"
#include "AISystem.h"
#include "DynamicColliderSystem.h"
#include "ParticleSystem.h"
#include "BattleSystem.h"
#include "ChatBox.h"
#include "ColliderRouting.h"
#include "EnvironmentSystem.h"
#include "Core/NavSystem.h"
#include "Core/Level.h"
#include "Core/Camera.h"
#include "Core/NavGraph.h"

class BattleLevel : public Level
{
public:
    BattleLevel(int lvlId_, FPSUtility &fpsUtility_, const Vector2<int>& size_);
    void enter() override;

    void receiveEvents(GAMEPLAY_EVENTS event, float scale_) override;

protected:
    void update() override;
    void draw() const override;

    void handleReset();

    template<typename... Components>
    void handleResetStaticHandler();

    template<typename Component>
    void handleResetHandler();

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

    entt::entity m_playerId = entt::null;
    entt::entity m_enemyId = entt::null;

    ColliderRoutesCollection m_cldRoutesCollection;
    LevelBuilder m_lvlBuilder;
};

#endif

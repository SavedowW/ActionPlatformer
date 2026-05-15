#include "EnemySystem.h"
#include "Hit.h"
#include "World.h"
#include "Core/Application.h"
#include "Core/CoreComponents.h"
#include "SM/StateMachine.hpp"
#include "Enemy1.h"
#include "Core/NavGraph.h"
#include "ResetHandlers.h"
#include "Core/Behavior.hpp"

EnemySystem::EnemySystem(entt::registry &reg_, NavSystem &navsys_, Camera &cam_, ParticleSystem &partsys_, const PlayerSystem &playerSystem_) :
    m_reg(reg_),
    m_animManager(Application::instance().m_animationManager),
    m_playersys{playerSystem_},
    m_navsys(navsys_),
    m_partsys(partsys_),
    m_cam(cam_)
{
}

entt::entity EnemySystem::makeEnemy()
{
    auto enemyId = m_reg.create();
    m_reg.emplace<HUDPoint>(enemyId, HUDPosRule::REL_TRANSFORM, Vector2{0, -16}, 16);

    const auto &trans = m_reg.emplace<ComponentTransform>(enemyId, Vector2{780, 470}, ORIENTATION::RIGHT);
    m_reg.emplace<ComponentReset<ComponentTransform>>(enemyId, trans.m_pos, trans.m_orientation);

    auto &phys = m_reg.emplace<ComponentPhysical>(enemyId);
    m_reg.emplace<ComponentResetStatic<ComponentPhysical>>(enemyId);
    phys.m_pushbox = Collider(Vector2{-15, -30}, Vector2{30, 30});
    phys.m_gravity = {0.0f, 0.2f};

    m_reg.emplace<PhysicalEvents>(enemyId);
    m_reg.emplace<BattleActor>(enemyId, BattleTeams::ENEMIES);

    auto &nav = m_reg.emplace<Navigatable>(enemyId);
    nav.m_traverseTraits = Traverse::makeSignature(true, TraverseTraits::WALK, TraverseTraits::JUMP,  TraverseTraits::FALL);
    nav.m_maxRange = 60.0f;
    nav.m_nodeTransitionRange = 15.0f; // Different transitions ranges for different traverse types?
    nav.m_checkIfGrounded = true;

    m_reg.emplace<World>(enemyId, m_reg, m_cam, m_partsys, m_navsys);
    m_reg.emplace<ComponentObstacleFallthrough>(enemyId);


    /*auto &ai = m_reg.emplace<ComponentAI>(enemyId); // TODO: reset
    ai.m_requestedState = static_cast<CharState>(Enemy1State::IDLE);
    ai.m_requestedOrientation = ORIENTATION::RIGHT;
    ai.m_navigationTarget = {430, 410};
    ai.m_chaseTarget = m_playerId;*/

    m_reg.emplace<HealthOwner>(enemyId, 3); // TODO: reset
    m_reg.emplace<HealthRendererCommonWRT>(enemyId, 3, Vector2{0.0f, -28.0f}); // TODO: reset

    auto &animrnd = m_reg.emplace<ComponentAnimationRenderable>(enemyId);
    m_reg.emplace<ComponentResetStatic<ComponentAnimationRenderable>>(enemyId);
    animrnd.m_drawOutline = true;
    m_reg.emplace<RenderLayer>(enemyId, 6);

    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Enemy1/idle"));
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Enemy1/float"));
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Enemy1/run"));
    animrnd.loadAnimation(m_animManager, m_animManager.getAnimID("Enemy1/prejump"));

    animrnd.m_currentAnimation = &animrnd.m_animations.at(m_animManager.getAnimID("Enemy1/idle"));
    animrnd.m_currentAnimation->reset();

    //auto &sm = m_reg.emplace<StateMachine>(enemyId);
    //m_reg.emplace<ComponentReset<StateMachine>>(enemyId).m_defaultStates = {static_cast<CharState>(Enemy1State::FLOAT)};

    return enemyId;
}

void EnemySystem::update()
{
    
}

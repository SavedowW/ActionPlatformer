#include "EnemySystem.h"
#include "CoreComponents.h"

EnemySystem::EnemySystem(entt::registry &reg_, Application &app_) :
    m_reg(reg_),
    m_animManager(*app_.getAnimationManager())
{
}

void EnemySystem::makeEnemy()
{
    auto enemyId = m_reg.create();
    m_reg.emplace<ComponentTransform>(enemyId, Vector2{170.0f, 300.0f}, ORIENTATION::RIGHT);
    auto &phys = m_reg.emplace<ComponentPhysical>(enemyId);
    m_reg.emplace<ComponentObstacleFallthrough>(enemyId);
    
    phys.m_pushbox = Collider(Vector2{0.0f, -15.0f}, Vector2{15.0f, 15.0f});
    phys.m_gravity = {0.0f, 0.2f};
}

void EnemySystem::update()
{
    
}

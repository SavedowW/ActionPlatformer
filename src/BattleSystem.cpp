#include "BattleSystem.h"
#include "StateMachine.h"

BattleSystem::BattleSystem(entt::registry &reg_, Application &app_) :
    m_reg(reg_), m_app(app_)
{
}

void BattleSystem::update()
{
    auto viewBtl = m_reg.view<BattleActor, StateMachine>();
    for (auto [idx, btl, sm] : viewBtl.each())
    {
        dynamic_cast<PhysicalState*>(sm.getRealCurrentState())->updateHurtboxes(btl);
    }
}

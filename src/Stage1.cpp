#include "Stage1.h"
#include "Application.h"

Stage1::Stage1(Application *application_, const Vector2<float>& size_, int lvlId_) :
    BattleLevel(application_, size_, lvlId_)
{
    m_background = std::make_unique<Stage1Background>(application_->getTextureManager());
}

void Stage1::update()
{
    BattleLevel::update();
}

void Stage1::enter()
{
    BattleLevel::enter();
}
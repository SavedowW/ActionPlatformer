#include "DebugPlayerWidget.h"

DebugPlayerWidget::DebugPlayerWidget(Application &application_, Camera &camera_, PlayableCharacter *pc_) :
    m_camera(camera_),
    m_textManager(*application_.getTextManager()),
    m_pc(pc_)
{
}

void DebugPlayerWidget::update()
{
}

void DebugPlayerWidget::draw(Renderer &renderer_, Camera &camera_)
{
    m_textManager.renderText("Player pos: " + utils::toString(m_pc->accessPos()), 0, {400, 1});
    m_textManager.renderText("Player vel: " + utils::toString(m_pc->accessPreEditVelocity()), 0, {400, 1 + 22 * 1});
    m_textManager.renderText(std::string("Player action: ") + m_pc->getCurrentActionName(), 0, {400, 1 + 22 * 2});
}

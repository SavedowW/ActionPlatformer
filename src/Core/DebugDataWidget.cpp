#include "DebugDataWidget.h"

DebugDataWidget::DebugDataWidget(Application &application_, Camera &camera_, int lvlId_, Vector2<float> lvlSize_, Uint32 &frameTime_) :
    m_camera(camera_),
    m_textManager(*application_.getTextManager()),
    m_lvlId(lvlId_),
    m_lvlSize(lvlSize_),
    m_frameTime(frameTime_)
{
}

void DebugDataWidget::update()
{
}

void DebugDataWidget::draw(Renderer &renderer_, Camera &camera_)
{
    m_textManager.renderText("[" + std::to_string(m_lvlId) + "] " + utils::toString(m_lvlSize), 0, {1, 1});
    m_textManager.renderText("Camera pos: " + utils::toString(camera_.getPos()), 0, {1, 1 + 22 * 1});
    m_textManager.renderText("Camera size: " + utils::toString(camera_.getSize()), 0, {1, 1 + 22 * 2});
    m_textManager.renderText("Camera scale: " + std::to_string(camera_.getScale()), 0, {1, 1 + 22 * 3});
    m_textManager.renderText("Real frame time (MS): " + std::to_string(m_frameTime), 0, {1, 1 + 22 * 4});
    m_textManager.renderText("Кириллица работает", 0, {1, 1 + 22 * 5});
}

#include "DebugDataWidget.h"

DebugDataWidget::DebugDataWidget(Application &application_, Camera &camera_, int lvlId_, Vector2<float> lvlSize_) :
    m_camera(camera_),
    m_textManager(*application_.getTextManager()),
    m_lvlId(lvlId_),
    m_lvlSize(lvlSize_)
{
    tex = m_textManager.renderStraight("Простой текст");
    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
}

void DebugDataWidget::update()
{
}

void DebugDataWidget::draw(Renderer &renderer_, Camera &camera_)
{
    m_textManager.renderText("[" + std::to_string(m_lvlId) + "] " + utils::toString(m_lvlSize), 0, {1, 1});
    m_textManager.renderText("Camera pos: " + utils::toString(camera_.getPos()), 0, {1, 1 + 15 * 1});
    m_textManager.renderText("Camera size: " + utils::toString(camera_.getSize()), 0, {1, 1 + 15 * 2});
    m_textManager.renderText("Camera scale: " + std::to_string(camera_.getScale()), 0, {1, 1 + 15 * 3});
    renderer_.renderTexture(tex, 10, 200, w, h);
}

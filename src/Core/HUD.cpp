#include "HUD.h"

HUD::HUD()
{

}

void HUD::addWidget(std::unique_ptr<Widget> widget_)
{
    m_widgets.push_back(std::move(widget_));
}

void HUD::clear()
{
    m_widgets.clear();
}

void HUD::update()
{
    for (auto &w : m_widgets)
        w->update();
}

void HUD::draw(Renderer &renderer_, Camera &camera_)
{
    for (auto &w : m_widgets)
        w->draw(renderer_, camera_);
}
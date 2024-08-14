#include "NavSystem.h"
#include "CoreComponents.h"

NavSystem::NavSystem(entt::registry &reg_, Application &app_, NavGraph &graph_) :
    m_reg(reg_),
    m_ren(*app_.getRenderer()),
    m_graph(graph_),
    m_textman(*app_.getTextManager())
{
}

void NavSystem::update()
{
    auto view = m_reg.view<ComponentTransform, Navigatable>();
    for (auto [idx, trans, nav] : view.each())
    {
        auto newCon = m_graph.findClosestConnection(trans.m_pos, nav.m_validTraitsOwnLocation);
        if (newCon.second <= nav.m_maxRange)
            nav.m_currentOwnConnection = newCon.first;
        else
            nav.m_currentOwnConnection = nullptr;
    }
}

void NavSystem::draw(Camera &cam_)
{
    auto view = m_reg.view<ComponentTransform, Navigatable>();
    for (auto [idx, trans, nav] : view.each())
    {
        if (nav.m_currentOwnConnection)
        {
            auto p1 = trans.m_pos;
            auto p2 = m_graph.getConnectionCenter(nav.m_currentOwnConnection);
            m_ren.drawLine(p1, p2, {255, 150, 100, 255}, cam_);

            auto range = m_graph.getDistToConnection(nav.m_currentOwnConnection, trans.m_pos);
            m_textman.renderText(std::to_string(range), 2, (p1 + p2) / 2.0f - Vector2{0.0f, 12.0f}, fonts::HOR_ALIGN::CENTER, &cam_);
        }
    }
}

NavPath::NavPath(NavGraph *graph_) :
    m_graph(graph_)
{
}

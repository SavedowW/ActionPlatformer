#pragma once
#include "ImmediateScreenLog.h"
#include "Application.h"

template<typename AlignerT>
ImmediateScreenLog<AlignerT>::ImmediateScreenLog(int font_, int offsetPerLine_, const Vector2<int> &origin_) :
    m_textManager(Application::instance().m_textManager),
    m_pos{origin_},
    m_font(font_),
    m_offsetPerLine(offsetPerLine_)
{
}

template<typename AlignerT>
void ImmediateScreenLog<AlignerT>::dumpLine(const std::string &record_)
{
    m_textManager.renderText<AlignerT>(record_, m_font, m_pos);
    m_pos.y += m_offsetPerLine;
}

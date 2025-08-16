#include "ImmediateScreenLog.h"
#include "Application.h"

ImmediateScreenLog::ImmediateScreenLog(int font_, fonts::HOR_ALIGN alignment_, int offsetPerLine_) :
    m_textManager(Application::instance().m_textManager),
    m_font(font_),
    m_alignment(alignment_),
    m_offsetPerLine(offsetPerLine_)
{
}
void ImmediateScreenLog::addRecord(const std::string &record_)
{
    m_records.push_back(record_);
}

void ImmediateScreenLog::dump(Vector2<float> origin_)
{
    for (auto &el : m_records)
    {
        m_textManager.renderText(el, m_font, origin_, m_alignment);
        origin_.y += m_offsetPerLine;
    }

    m_records.clear();
}

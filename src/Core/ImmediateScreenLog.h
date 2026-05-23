#pragma once
#include "TextManager.h"

template<typename AlignerT>
class ImmediateScreenLog
{
public:
    ImmediateScreenLog(Fonts font_, int offsetPerLine_, const Vector2<int> &origin_);
    void dumpLine(const std::string &record_);

private:
    TextManager &m_textManager;
    Vector2<int> m_pos;
    const Fonts m_font;
    const int m_offsetPerLine;
};

#ifndef COMMON_SCREEN_LOG_H_
#define COMMON_SCREEN_LOG_H_

#include "Application.h"
#include "GameData.h"
#include "TextManager.h"

class ImmediateScreenLog
{
public:
    ImmediateScreenLog(Application &application_, int font_, fonts::HOR_ALIGN alignment_, int offsetPerLine_);
    void addRecord(const std::string &record_);
    void dump(Vector2<float> origin_);
    ~ImmediateScreenLog() = default;

private:
    TextManager &m_textManager;
    std::vector<std::string> m_records;
    int m_font;
    fonts::HOR_ALIGN m_alignment;
    int m_offsetPerLine;
};

#endif
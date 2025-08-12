#include "LocalizationGen.h"
#include "Application.h"
#include "FilesystemUtils.h"
#include <nlohmann/json.hpp>

ll::LocalMap ll::m_stringsen;
ll::LocalMap ll::m_stringsru;
ll::LocalMap *ll::m_currentStrings = &ll::m_stringsen;

void ll::setLang(const std::string &lang_)
{
    if (lang_ == "ru")
        m_currentStrings = &m_stringsru;
    else
        m_currentStrings = &m_stringsen;
}

void ll::load()
{
    {
        std::ifstream jsonIn(Filesystem::getRootDirectory() + "Localization/en/strings.json");
        auto jsonData = nlohmann::json::parse(jsonIn);
        m_stringsen[0] = jsonData["dbg_localization"];
        m_stringsen[1] = jsonData["test_dlg1"];
        m_stringsen[2] = jsonData["test_dlg2"];
        m_stringsen[3] = jsonData["test_dlg3"];
        m_stringsen[4] = jsonData["test_dlg4"];
        m_stringsen[5] = jsonData["test_dlg5"];
    }
    {
        std::ifstream jsonIn(Filesystem::getRootDirectory() + "Localization/ru/strings.json");
        auto jsonData = nlohmann::json::parse(jsonIn);
        m_stringsru[0] = jsonData["dbg_localization"];
        m_stringsru[1] = jsonData["test_dlg1"];
        m_stringsru[2] = jsonData["test_dlg2"];
        m_stringsru[3] = jsonData["test_dlg3"];
        m_stringsru[4] = jsonData["test_dlg4"];
        m_stringsru[5] = jsonData["test_dlg5"];
    }
}

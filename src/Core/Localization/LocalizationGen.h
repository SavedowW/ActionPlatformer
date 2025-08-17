#ifndef LOCALIZATION_GEN_H_
#define LOCALIZATION_GEN_H_
#include <string>
#include <array>

#define GENERATE_LOCALIZED_KEY(key, id) \
    static constexpr std::string key() \
    { \
        return (*m_currentStrings)[id]; \
    }

struct ll
{
public:
    static void setLang(const std::string &lang_);
    static void load();

    GENERATE_LOCALIZED_KEY(dbg_localization, 0)
    GENERATE_LOCALIZED_KEY(test_dlg1, 1)
    GENERATE_LOCALIZED_KEY(test_dlg2, 2)
    GENERATE_LOCALIZED_KEY(test_dlg3, 3)
    GENERATE_LOCALIZED_KEY(test_dlg4, 4)
    GENERATE_LOCALIZED_KEY(test_dlg5, 5)

private:
    using LocalMap = std::array<std::string, 6>;
    static LocalMap m_stringsru;
    static LocalMap m_stringsen;
    static LocalMap *m_currentStrings;
};

#endif

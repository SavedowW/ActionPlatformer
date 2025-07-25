#include "Configuration.h"
#include "FilesystemUtils.h"

Configuration::Configuration(const std::string &configName_, bool createIfNotExists_) :
    m_path(Filesystem::getRootDirectory() + "Configs/" + configName_ + ".json")
{
    std::ifstream injson(m_path);
    if (injson.is_open())
    {
        m_data = nlohmann::json::parse(injson);
    }
    else
    {
        if (createIfNotExists_)
        {
            std::cout << "Creating config at \"" << m_path << "\"" << std::endl;
            std::ofstream of(m_path);
            of << "{}\n";
        }
        else
        {
            std::cout << "Config \"" << m_path << "\" doesn't exist, not loading it" << std::endl;
        }
    }
}

nlohmann::json &Configuration::write()
{
    return m_data;
}

const nlohmann::json &Configuration::read() const
{
    return m_data;
}

void Configuration::save()
{
    std::ofstream fout(m_path);
    fout << m_data.dump(4);
}

ConfigurationManager::ConfigurationManager() :
    m_debug("debug", false),
    m_settings("settings", true)
{
}

ConfigurationManager &ConfigurationManager::instance()
{
    static ConfigurationManager mgmt;
    return mgmt;
}

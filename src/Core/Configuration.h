#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

class Configuration
{
public:
    // "config" => "C:/Game/Configs/config.json"
    Configuration(const std::string &configName_, bool createIfNotExists_);
    nlohmann::json &write();
    const nlohmann::json &read() const;

    void save();

private:
    nlohmann::json m_data;
    bool m_trueDirty = false;
    const std::string m_path;
};

class ConfigurationManager
{
public:
    Configuration m_debug;
    Configuration m_settings;

    static ConfigurationManager &instance();

private:
    ConfigurationManager();

};

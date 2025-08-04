#include "Configuration.h"
#include "FilesystemUtils.h"
#include "GameData.h"

template <bool READ_ONLY>
ConfigurationView<READ_ONLY>::ConfigurationView(nlohmann::json &dataRef_, bool &dirtyRef_) :
    m_dataRef(dataRef_),
    m_dirtyRef(dirtyRef_)
{
}

template <bool READ_ONLY>
ConfigurationView<READ_ONLY> ConfigurationView<READ_ONLY>::operator[](const std::string &field_)
{
    try
    {
        return {m_dataRef.at(field_), m_dirtyRef};
    }
    catch (std::exception &ex_)
    {
        if constexpr (READ_ONLY)
        {
            static Configuration<true> emptycfg;
            return (ConfigurationView<true>&)(emptycfg);
        }
        else
            return {m_dataRef[field_] = nlohmann::json::object(), m_dirtyRef};
    }
}

template <bool READ_ONLY>
Configuration<READ_ONLY>::Configuration() :
    ConfigurationView<READ_ONLY>(m_data, m_dirty)
{
}

template <bool READ_ONLY>
Configuration<READ_ONLY>::Configuration(const std::string &configName_) : ConfigurationView<READ_ONLY>(m_data, m_dirty),
                                                                          m_path(Filesystem::getRootDirectory() + "Configs/" + configName_ + ".json")
{
    std::ifstream injson(m_path);
    if (injson.is_open())
    {
        try
        {
            m_data = nlohmann::json::parse(injson);
            std::cout << "Successfully loaded \"" << m_path << "\"" << std::endl;
        }
        catch (std::exception &ex_)
        {
            injson.close();

            if constexpr (READ_ONLY)
            {
                std::cout << "Failed to parse \"" << m_path << "\", most likely it's broken, leaving it this way" << std::endl;
            }
            else
            {
                std::cout << "Failed to parse \"" << m_path << "\", most likely it's broken. Turning it empty." << std::endl;
                std::ofstream of(m_path);
                of << "{}\n";
            }
        }
    }
    else
    {
        if constexpr (READ_ONLY)
        {
            std::cout << "No config at \"" << m_path << "\", leaving it this way" << std::endl;
        }
        else
        {
            std::cout << "Creating config at \"" << m_path << "\"" << std::endl;
            std::ofstream of(m_path);
            of << "{}\n";
        }
    }
}

template <bool READ_ONLY>
void Configuration<READ_ONLY>::save() requires (!READ_ONLY)
{
    if (m_dirty)
    {
        try
        {
            m_dirty = false;
            std::ofstream fout(m_path);
            fout << m_data.dump(4) << "\n";
        }
        catch (std::exception &ex_)
        {
            std::cout << "WARNING: failed to update config file for \"" << m_path << "\"" << std::endl;
        }
    }
}

ConfigurationManager::ConfigurationManager() :
    m_debugConf("debug"),
    m_settings("settings")
{
    m_debug.m_drawColliders = m_debugConf["video"]["draw_colliders"].readOrDefault(gamedata::debug_defaults::drawColliders);
    m_debug.m_drawFocusAreas = m_debugConf["video"]["draw_focus_areas"].readOrDefault(gamedata::debug_defaults::drawFocusAreas);
    m_debug.m_drawCameraOffset = m_debugConf["video"]["draw_camera_offset"].readOrDefault(gamedata::debug_defaults::drawCameraOffset);
    m_debug.m_drawNpcDebug = m_debugConf["video"]["draw_npc_debug"].readOrDefault(gamedata::debug_defaults::drawNpcDebug);
    m_debug.m_drawCurrentConnection = m_debugConf["video"]["draw_current_connection"].readOrDefault(gamedata::debug_defaults::drawCurrentConnection);
    m_debug.m_drawNavGraph = m_debugConf["video"]["draw_nav_graph"].readOrDefault(gamedata::debug_defaults::drawNavGraph);
    m_debug.m_drawTransforms = m_debugConf["video"]["draw_transforms"].readOrDefault(gamedata::debug_defaults::drawTransforms);
    m_debug.m_drawDebugTextures = m_debugConf["video"]["draw_debug_textures"].readOrDefault(gamedata::debug_defaults::drawDebugTextures);
    m_debug.m_drawHealthPos = m_debugConf["video"]["draw_health_pos"].readOrDefault(gamedata::debug_defaults::drawHealthPos);
    m_debug.m_drawColliderRoutes = m_debugConf["video"]["draw_collider_routes"].readOrDefault(gamedata::debug_defaults::drawColliderRoutes);
    m_debug.m_debugPathDisplay = m_debugConf["video"]["path_display"].readOrDefault(gamedata::debug_defaults::debugPathDisplay);
}

ConfigurationManager &ConfigurationManager::instance()
{
    static ConfigurationManager mgmt;
    return mgmt;
}

template class ConfigurationView<false>;
template class ConfigurationView<true>;
template class Configuration<false>;
template class Configuration<true>;

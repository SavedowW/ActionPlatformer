#pragma once
#include "JsonUtils.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

template <bool READ_ONLY>
class ConfigurationView
{
public:
    ConfigurationView(nlohmann::json &dataRef_, bool &dirtyRef_);
    ConfigurationView<READ_ONLY> operator[](const std::string &field_);

    template<typename T>
    T readOrSet(const T &default_);

    template<typename T>
    T readOrDefault(const T &default_) const;

private:
    nlohmann::json &m_dataRef;
    bool &m_dirtyRef;
};

template <bool READ_ONLY>
class Configuration : public ConfigurationView<READ_ONLY>
{
public:
    // "config" => "C:/Game/Configs/config.json"
    Configuration();
    Configuration(const std::string &configName_);
    nlohmann::json &write();
    const nlohmann::json &read() const;

    void save();

private:
    nlohmann::json m_data;
    bool m_dirty = false;
    const std::string m_path;
};

class ConfigurationManager
{
public:
    Configuration<true> m_settings;
    struct DebugConf
    {
        bool m_drawColliders;
        bool m_drawFocusAreas;
        bool m_drawCameraOffset;
        bool m_drawNpcDebug;
        bool m_drawCurrentConnection;
        bool m_drawNavGraph;
        bool m_drawTransforms;
        bool m_drawDebugTextures;
        bool m_drawHealthPos;
        bool m_drawColliderRoutes;
        uint32_t m_debugPathDisplay;
    } m_debug;

static ConfigurationManager &instance();

private:
    Configuration<false> m_debugConf;
    ConfigurationManager();

};


template <bool READ_ONLY>
template <typename T>
inline T ConfigurationView<READ_ONLY>::readOrSet(const T &default_)
{
    static_assert(!READ_ONLY);

    try 
    {
        return m_dataRef.template get<T>();
    }
    catch (std::exception &ex_)
    {
        m_dataRef = default_;
        m_dirtyRef = true;
        return default_;
    }
}

template <bool READ_ONLY>
template <typename T>
inline T ConfigurationView<READ_ONLY>::readOrDefault(const T &default_) const
{
    try
    {
        return m_dataRef.template get<T>();
    }
    catch (std::exception &ex_)
    {
        return default_;
    }
}

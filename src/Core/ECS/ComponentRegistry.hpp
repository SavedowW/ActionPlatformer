#pragma once
#include "ComponentExtractor.h"
#include "ComponentRegistry.h"
#include "entt/core/type_info.hpp"
#include <iostream>

template<typename T>
void ComponentRegistry::initExtractorFactory()
{
    const auto id = entt::type_hash<T>();
    const std::string name{entt::type_name<T>{}.value()};
    if (m_extractors.contains(name))
        std::cout << "Warning: component registry already has factory for \"" << entt::type_name<T>{}.value() << '\"' << std::endl;
    else
    {
        std::cout << "Adding factory for component extractor \"" << entt::type_name<T>{}.value() << '\"' << std::endl;
        m_extractors.emplace(name, std::pair{id, [](){
            return std::make_unique<ComponentExtractor<T>>();
        }});
    }
}

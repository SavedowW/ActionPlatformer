#pragma once
#include "ComponentExtractor.h"
#include <functional>

class ComponentRegistry
{
public:
    template<typename T>
    void initExtractorFactory();

    std::pair<entt::id_type, std::unique_ptr<IComponentExtractor>> makeExtractor(const std::string &name_) const;

private:
    std::unordered_map<std::string, std::pair<entt::id_type, std::function<std::unique_ptr<IComponentExtractor>()>>> m_extractors;
};

#pragma once
#include "ComponentExtractor.h"
#include <functional>

/**
 *  Register a bunch of component types
 *  After that, create their extractors upon request
 *  Extractor's user should know it's actual type
 */
class ComponentRegistry
{
public:
    template<typename T>
    void initExtractorFactory();

    std::pair<entt::id_type, std::unique_ptr<IComponentExtractor>> makeExtractor(const std::string &name_) const;

    friend std::ostream &operator<<(std::ostream &os_, const ComponentRegistry &reg_);

private:
    /**
     *  k: 
     *   type name (string)
     *  v:
     *   pair:
     *    l - type hash
     *    r - extractor constructor
     */
    std::unordered_map<std::string, std::pair<entt::id_type, std::function<std::unique_ptr<IComponentExtractor>()>>> m_extractors;
};

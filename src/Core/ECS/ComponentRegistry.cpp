#include "ComponentRegistry.hpp"
#include <stdexcept>
#include "../Logger.h"

std::pair<entt::id_type, std::unique_ptr<IComponentExtractor>> ComponentRegistry::makeExtractor(const std::string &name_) const
{
    try
    {
        const auto &el = m_extractors.at(name_);
        return std::pair{el.first, el.second()};
    }
    catch_extend("While creating extractor \"" + name_ + '"')
}

#include "ComponentRegistry.hpp"
#include <ios>
#include <stdexcept>
#include <iomanip>
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

std::ostream &operator<<(std::ostream &os_, const ComponentRegistry &reg_)
{
    const auto size = reg_.m_extractors.size();
    os_ << "ComponentRegistry [" << size << "]";
    if (!size)
        return os_;

    os_ << ':';
    for (const auto &el : reg_.m_extractors)
        os_ << "\n  " << std::setw(32) << el.first << " (" << std::hex << std::showbase << el.second.first << ")";

    os_ << std::dec;

    return os_;
}

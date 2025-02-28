#ifndef JSON_UTILS_H_
#define JSON_UTILS_H_
#include "EnumMapping.hpp"
#include <nlohmann/json.hpp>

namespace utils
{
    template<typename KeyT, typename ValueT>
    void exportMap(nlohmann::json &out_, const std::map<KeyT, ValueT> &map_)
    {
        for (const auto &el : map_)
        {
            out_[serialize(el.first)] = serialize(el.second);
        }
    }

    // Returns true if there were repeated values
    template<typename KeyT, typename ValueT>
    bool importMapEnsureUnique(nlohmann::json &out_, std::map<KeyT, ValueT> &map_, bool hadDuplicated_)
    {
        std::set<ValueT> values;
        for (auto &el : out_.items())
        {
            auto key = deserialize<KeyT>(el.key());
            auto value = deserialize<ValueT>(el.value());

            if (values.contains(value))
            {
                hadDuplicated_ = true;
                continue;
            }
            else
                values.insert(value);

            map_[key] = value;
        }

        return hadDuplicated_;
    }
}

#endif

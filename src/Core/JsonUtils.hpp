#ifndef JSON_UTILS_H_
#define JSON_UTILS_H_
#include "EnumMapping.hpp"
#include "Vector2.h"
#include <set>
#include <nlohmann/json.hpp>

template<typename T>
void to_json(nlohmann::json& j_, const Vector2<T>& v_)
{
    j_ = nlohmann::json{ {"x", v_.x}, {"y", v_.y} };
}

template<typename T>
void from_json(const nlohmann::json& j, Vector2<T>& v_)
{
    j.at("x").get_to(v_.x);
    j.at("y").get_to(v_.y);
}

namespace utils
{
    template<typename T>
    T tryClaim(const nlohmann::json &src_, const std::string &field_, const T &default_)
    {
        try
        {
            if (src_.contains(field_))
                return src_[field_];
        }
        catch(std::exception &ex_)
        {
            // Property is invalid
        }

        return default_;
    }

    template<typename T>
    Vector2<T> tryClaimVector(const nlohmann::json &src_, const std::initializer_list<std::string> &path_, const Vector2<T> &default_)
    {
        try
        {
            const nlohmann::json *current = &src_;;
            for (const auto &el : path_)
            {
                if (!current->contains(el))
                    return default_;

                current = &((*current)[el]);
            }

            if (current->contains("x") && current->contains("y"))
                return Vector2<T>{(*current)["x"], (*current)["y"]};
        }
        catch(std::exception &ex_)
        {
            // Property is invalid
        }

        return default_;
    }

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

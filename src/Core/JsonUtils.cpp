#include "JsonUtils.hpp"
#include "Timer.h"

namespace utils {

    template<>
    Time::NS tryClaim<Time::NS>(const nlohmann::json &src_, const std::string &field_, const Time::NS &default_)
    {
        if (!src_.contains(field_))
            return default_;

        try
        {
            const auto &val = src_[field_];
            if (val.is_number())
                return Time::fromFrames(val);

            return Time::deserialize(val);
        }
        catch(const std::exception &)
        {
            // Property is invalid
            return default_;
        }
    }

}

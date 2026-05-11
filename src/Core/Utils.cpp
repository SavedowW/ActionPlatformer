#include "Utils.hpp"
#include <sstream>

namespace utils
{
    std::string getIntend(size_t intend_)
    {
        return std::string(intend_, ' ');
    }

    std::string wrap(const std::string &src_)
    {
        return "\"" + src_ + "\"";
    }
    
    std::vector<std::string> tokenize(const std::string &src_, const char splitter_)
    {
        std::stringstream ss(src_);
        std::vector<std::string> res;

        while (ss.good())
        {
            std::string substr;
            std::getline(ss, substr, splitter_);
            res.push_back(substr);
        }

        return res;
    }

    std::string padToRight(const size_t &len_, const std::string &line_)
    {
        if (line_.size() >= len_)
            return line_;

        return std::string(len_ - line_.size(), ' ') + line_;
    }
}

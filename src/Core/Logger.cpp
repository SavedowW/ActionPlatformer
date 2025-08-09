#include "Logger.h"

std::string utils::prettifyFunction(const std::string &functionName_)
{
    auto i = functionName_.size() - 1;
    while (functionName_[i--] != ')');
    int depth = 1;
    while (depth != 0)
    {
        if (functionName_[i] == ')')
            depth++;
        else if (functionName_[i] == '(')
            depth--;
        
        i--;
    }

    auto newname = functionName_.substr(0, i + 1);
    while (auto pos = newname.find("__cdecl"))
    {
        if (pos == std::string::npos)
            break;
        newname = newname.substr(pos + 7, newname.size() - pos - 7);
    }

    return utils::cutBoundingSpaces(newname);
}

std::string utils::cutBoundingSpaces(const std::string &functionName_)
{
    auto dst = functionName_;

    while (!dst.empty() && dst.front() == ' ')
        dst.erase(dst.begin());

    while (!dst.empty() && dst.back() == ' ')
        dst.erase(dst.begin() + dst.size() - 1);

    return dst;
}

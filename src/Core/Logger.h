#ifndef LOGGER_H_
#define LOGGER_H_
#include <string>

#define catch_extend(args) \
catch (std::exception &ex_) \
{ \
    std::stringstream ss; \
    ss << args << "\n > " << ex_.what(); \
    throw std::runtime_error(ss.str()); \
}

namespace utils
{
    std::string prettifyFunction(const std::string &functionName_);
    std::string cutBoundingSpaces(const std::string &functionName_);
}


// TODO:

#endif

#ifndef ENUM_MAPPING_H_
#define ENUM_MAPPING_H_

#include <string>
#include <map>

template<typename T>
const std::map<T, std::string> makeDirectMap()
{
    throw std::string("makeDirectMap is not implemented for type ") + typeid(T).name();
}

template<typename T>
const std::map<std::string, T> makeReversedMap()
{
    throw std::string("makeReversedMap is not implemented for type ") + typeid(T).name();
}

template<typename T>
const std::string serialize(const T &value_)
{
    static const std::map<T, std::string> m(makeDirectMap<T>());

    return m.at(value_);
}

template<typename T>
const bool isSerializable(const T &value_)
{
    static const std::map<T, std::string> m(makeDirectMap<T>());

    return m.contains(value_);
}

template<typename T>
const T deserialize(const std::string &value_)
{
    static const std::map<std::string, T> m(makeReversedMap<T>());

    return m.at(value_);
}

#define SERIALIZE_ENUM(EnumType, ...) \
template<> \
inline const std::map<EnumType, std::string> makeDirectMap<EnumType>() \
{ \
    const static std::map<EnumType, std::string> m##__VA_ARGS__ ; \
    return m; \
} \
template<> \
inline const std::map<std::string, EnumType> makeReversedMap<EnumType>() \
{ \
    const static std::map<EnumType, std::string> m##__VA_ARGS__; \
    std::map<std::string, EnumType> reversed; \
        for (const auto &el : m) \
            reversed[el.second] = el.first; \
        return reversed; \
}

#define ENUM_INIT(EnumType, Value, Line) {EnumType::Value, Line}
#define ENUM_INIT_NODOTS(Value, Line) {Value, Line}
#define ENUM_AUTO(EnumType, Value) {EnumType::Value, #Value}

#endif
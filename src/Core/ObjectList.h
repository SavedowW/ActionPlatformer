#ifndef OBJECT_LIST_H_
#define OBJECT_LIST_H_
#include "EnumMapping.hpp"

using ObjectClassT = int;
enum class ObjectClass : ObjectClassT
{
    GRASS_TOUCHABLE,
    NONE
};

SERIALIZE_ENUM(ObjectClass, {
    ENUM_AUTO(ObjectClass, GRASS_TOUCHABLE),
    ENUM_AUTO(ObjectClass, NONE),
})

#endif

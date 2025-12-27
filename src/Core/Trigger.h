#ifndef TRIGGER_H_
#define TRIGGER_H_
#include "RectCollider.h"

struct Trigger : public Collider
{
    Trigger(float nx = 0, float ny = 0, float nw = 0, float nh = 0);

    template<typename T1, typename T2>
    constexpr inline Trigger(const Vector2<T1> &pos_, const Vector2<T2> &size_) :
        Collider(pos_, size_)
    {
    }

    using TagType = uint64_t;
    enum class Tag : TagType
    {
        NONE = 0,
        ClingArea = 1,
        LEFT = 2,
        RIGHT = 4
    } m_tag = Tag::NONE;

    Trigger &operator|=(const Tag &rhs_);
    bool operator&(const Tag &rhs_) const;
};

Trigger::Tag operator|(const Trigger::Tag &lhs_, const Trigger::Tag &rhs_);
Trigger::Tag &operator|=(Trigger::Tag &lhs_, const Trigger::Tag &rhs_);
Trigger::Tag operator&(const Trigger::Tag &lhs_, const Trigger::Tag &rhs_);

#endif
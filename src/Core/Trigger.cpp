#include "Trigger.h"

Trigger::Tag operator|(const Trigger::Tag &lhs_, const Trigger::Tag &rhs_)
{
    return Trigger::Tag(Trigger::TagType(lhs_) | Trigger::TagType(rhs_));
}

Trigger::Tag &operator|=(Trigger::Tag &lhs_, const Trigger::Tag &rhs_)
{
    lhs_ = lhs_ | rhs_;
    return lhs_;
}

Trigger::Tag operator&(const Trigger::Tag &lhs_, const Trigger::Tag &rhs_)
{
    return Trigger::Tag(Trigger::TagType(lhs_) & Trigger::TagType(rhs_));
}

Trigger::Trigger(float nx, float ny, float nw, float nh) :
    Collider(Vector2{nx + nw / 2.0f, ny + nh / 2.0f}, Vector2{nw / 2.0f, nh / 2.0f})
{
}

Trigger &Trigger::operator|=(const Tag &rhs_)
{
    m_tag |= rhs_;
    return *this;
}

bool Trigger::operator&(const Tag &rhs_) const
{
    return TagType(m_tag & rhs_);
}

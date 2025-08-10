#include "Behavior.hpp"

Behavior::NodeBase::NodeBase(const std::string &name_)  :
    m_name(name_)
{
}

void Behavior::NodeBase::leave()
{
}

std::string Behavior::NodeBase::stringify(size_t intend_) const
{
    return std::string(intend_, ' ') + describeSelf() + "\n";
}

std::string Behavior::NodeBase::describeSelf() const
{
    return m_name;
}


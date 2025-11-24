#include "ComponentExtractor.h"

template<typename T>
void ComponentExtractor<T>::select(entt::registry &reg_, entt::entity ent_)
{
    m_comp = reg_.try_get<T>(ent_);
    assert(m_comp != 0);
}

template<typename T>
T &ComponentExtractor<T>::extract() const
{
    return *m_comp;
}

void ComponentViewer::addExtractor(entt::id_type typeId_, std::unique_ptr<IComponentExtractor> &&extractor_)
{
    m_extractors.emplace(typeId_, std::move(extractor_));
}

void ComponentViewer::select(entt::registry &reg_, entt::entity ent_)
{
    for (auto &el : m_extractors)
        el.second->select(reg_, ent_);
}

template<typename T>
T &ComponentViewer::get()
{
    return dynamic_cast<ComponentExtractor<T>&>(*m_extractors.at(entt::type_hash<T>())).extract();
}

template<typename T>
const T &ComponentViewer::get() const
{
    return dynamic_cast<ComponentExtractor<T>&>(*m_extractors.at(entt::type_hash<T>())).extract();
}

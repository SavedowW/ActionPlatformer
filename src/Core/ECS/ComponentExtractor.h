#pragma once
#include "entt/entt.hpp"

class IComponentExtractor
{
public:
    virtual void select(entt::registry &reg_, entt::entity ent_) = 0;
    virtual ~IComponentExtractor() = default;

protected:
};

template<typename T>
class ComponentExtractor : public IComponentExtractor
{
public:
    void select(entt::registry &reg_, entt::entity ent_) override;
    T &extract() const;

private:
    T *m_comp = nullptr;
};

class ComponentViewer
{
public:
    void addExtractor(entt::id_type typeId_, std::unique_ptr<IComponentExtractor> &&extractor_);
    void select(entt::registry &reg_, entt::entity ent_);

    template<typename T>
    T &get() const;

    template<typename T>
    const T &cget() const;

private:
    std::unordered_map<entt::id_type, std::unique_ptr<IComponentExtractor>> m_extractors;
};

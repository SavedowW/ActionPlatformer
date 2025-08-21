#ifndef STATE_MACHINE_EX_H_
#define STATE_MACHINE_EX_H_
#include "World.h"
#include "Core/TypeList.hpp"
#include "Core/CoreComponents.h"
#include <cstdint>
#include <iostream>
#include <type_traits>

enum class EORIENTATION : int8_t
{
    UNSPECIFIED = 0b00,
    RIGHT =       0b01,
    LEFT =        0b10,
    ALL =         0b11
};

template<typename T>
void dumpType();

template<typename T>
constexpr auto collectAllDependencies();

// Take unique components, return all their dependencies (at Ty::Dependencies) in 1 list
template<typename T, typename... Ty>
constexpr auto collectAllDependencies() requires (sizeof...(Ty) > 0);

// Take type list, return type list with only unique types
template<typename... Ts>
constexpr auto getUniqueFromDeps(TypeList<Ts...>);

// Take list of types, get only unique dependencies (at Ts::Dependencies)
template<typename... Ts>
using getUniqueDependencies = decltype(getUniqueFromDeps(collectAllDependencies<Ts...>()));


struct UpdatePos
{
    int x = 0, y = 0;

    using Dependencies = TypeList<ComponentTransform>;
    void update(ComponentTransform&) const;
};

struct UpdateVelocity
{
    float x = 0.f, y = 0.f;

    using Dependencies = TypeList<ComponentPhysical, ComponentTransform>;
    void update(ComponentPhysical&, ComponentTransform&) const;
};

struct ChangeAnim
{
    ResID anim = 0;

    using Dependencies = TypeList<ComponentTransform, ComponentPhysical, ComponentAnimationRenderable>;
    void update(ComponentTransform&, ComponentPhysical&, ComponentAnimationRenderable&) const;
};

template<typename UpdArg>
class ICompoundState
{
public:
    ICompoundState() = default;
    virtual ~ICompoundState() = default;
    ICompoundState(const ICompoundState&) = default;
    ICompoundState(ICompoundState&&) noexcept = default;
    ICompoundState &operator=(const ICompoundState&) = default;
    ICompoundState &operator=(ICompoundState&&) noexcept = default;
    virtual void dump(std::ostream& out_, int indent_) const = 0;

    virtual void update(UpdArg) const = 0;
};

template<typename UpdArg, typename T>
class StateAdapter : public T, public ICompoundState<UpdArg>
{
public:
    template<typename... Args>
    StateAdapter(Args&&... args_);

    void dump(std::ostream& out_, int indent_) const override;
    void update(UpdArg arg_) const override;
};

template<typename... Args>
class CompoundPipe
{

};

template<typename... TComponents>
class CompoundState
{
public:
    using Dependencies = getUniqueDependencies<TComponents...>;

    CompoundState(TComponents&&... components_);

    CompoundState(const CompoundState&) = delete;
    CompoundState(CompoundState&&) noexcept = default;
    CompoundState &operator=(const CompoundState&) = delete;
    CompoundState &operator=(CompoundState&&) noexcept = default;

    void update(Dependencies::toTupleRefs dependencies_) const;


    void dump(std::ostream& out_, int indent_) const;

protected:
    std::tuple<TComponents...> m_components;
};

template<typename... TStates>
class EStateMachine
{
public:
    using Dependencies = getUniqueDependencies<TStates...>;

    struct Iterator
    {
        Iterator(const EStateMachine<TStates...> &machine_);

        const ICompoundState<typename Dependencies::toTupleRefs> *m_currentState = nullptr;
    };

    EStateMachine(std::string &&name_, TStates&&... states_);

    void dump(std::ostream& out_, int indent_) const;

    void update(Iterator &it_, EntityAnywhere owner_) const;

    auto getInitialState() const;


protected:
    // Possible to use simple tuple
    std::vector<std::unique_ptr<ICompoundState<typename Dependencies::toTupleRefs>>> m_states;
    std::string m_name;
};

#endif

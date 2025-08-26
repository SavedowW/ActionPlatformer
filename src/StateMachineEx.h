#ifndef STATE_MACHINE_EX_H_
#define STATE_MACHINE_EX_H_
#include "World.h"
#include "Core/CallChain.h"
#include "Core/TypeList.hpp"
#include "Core/CoreComponents.h"
#include "entt/entity/fwd.hpp"
#include <cstdint>
#include <iostream>

enum class EORIENTATION : int8_t
{
    UNSPECIFIED = 0b00,
    RIGHT =       0b01,
    LEFT =        0b10,
    ALL =         0b11
};


// Component of a CompoundState
// Represents it's update call
// Each component is called every time, callable::EachResolved resolves dependencies
template<typename... TCallable>
struct UpdateBody : public callable::EachResolved<TCallable...>
{
public:
    using Dependencies = getUniqueDependencies<TCallable...>;
    UpdateBody(TCallable&&... callables_);
    
private:
    using BodyType = callable::EachResolved<TCallable...>;
};

// Component of a CompoundState
// Represents it's incoming pipe to which other states connect
// Each component is called every time, callable::EachResolved resolves dependencies
template<typename... TCallable>
class CompoundInPipe : public callable::EachResolved<TCallable...>
{
public:
    using Dependencies = getUniqueDependencies<TCallable...>;
    CompoundInPipe(TCallable&&... callables_);

private:
    using BodyType = callable::EachResolved<TCallable...>;
};

// Component of a CompoundState
// Represents it's outgoing pipe, connected to a certain incoming pipe
// Condition is a condition to enter the state (inputs, orientation, etc)
// Rule is a logic to be executed before transition if the condition is fulfilled
template<typename ConditionChain, typename RuleChain>
class CompoundOutPipe
{
public:
    using Dependencies = getUniqueDependencies<typename ConditionChain::Dependencies, typename RuleChain::Dependencies>;
    CompoundOutPipe(ConditionChain &&conditionChain_, RuleChain &&ruleChain_);

    template<typename... Components>
    auto condition(std::tuple<Components&...> &args_);

private:
    ConditionChain m_condition;
    RuleChain m_rule;
};

template<typename... TPipes>
constexpr auto InPipeSet(TPipes&&... pipes_);


template<typename T>
void dumpType();

// Specialization in case of an empty typelist
//template<typename... Ts> requires (sizeof...(Ts) == 0)
//using getUniqueDependencies = decltype(getUniqueFromDeps(collectAllDependencies<Ts...>()));


// Placeholders
struct UpdatePos {
    int x = 0, y = 0;

    using Dependencies = TypeList<ComponentTransform>;
    void update(ComponentTransform&) const;
};
struct UpdateVelocity {
    float x = 0.f, y = 0.f;

    using Dependencies = TypeList<ComponentPhysical, ComponentTransform>;
    void update(ComponentPhysical&, ComponentTransform&) const;
};
struct ChangeAnim {
    ResID anim = 0;

    using Dependencies = TypeList<ComponentTransform, ComponentPhysical, ComponentAnimationRenderable>;
    void update(const ComponentTransform&, const ComponentPhysical&, ComponentAnimationRenderable&) const;
};

// Interface for a compound state
// Shouldn't be inherited from by anything else
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

    virtual void update(UpdArg) = 0;
};

// Hides actual arguments for T from interface
//    and resolves dependencies to only the required ones
template<typename UpdArg, typename T>
class StateAdapter : public T, public ICompoundState<UpdArg>
{
public:
    template<typename... Args>
    StateAdapter(Args&&... args_);

    void dump(std::ostream& out_, int indent_) const override;
    void update(UpdArg arg_) override;
};

template<typename TUpdateBody, typename TInPipes>
class CompoundState
{
public:
    using Dependencies = typename TUpdateBody::Dependencies;

    CompoundState(TUpdateBody &&updateBody_, TInPipes &&inPipes_);

    CompoundState(const CompoundState&) = delete;
    CompoundState(CompoundState&&) noexcept = default;
    CompoundState &operator=(const CompoundState&) = delete;
    CompoundState &operator=(CompoundState&&) noexcept = default;

    void update(Dependencies::toTupleRefs dependencies_);

    void dump(std::ostream& out_, int indent_) const;

protected:
    TUpdateBody m_updateBody;
    TInPipes m_inPipes;
};

template<typename... TStates>
class EStateMachine
{
public:
    using Dependencies = getUniqueDependencies<TStates...>;

    struct Iterator
    {
        Iterator(EStateMachine<TStates...> &machine_);

        ICompoundState<typename Dependencies::toTupleRefs> *m_currentState = nullptr;
    };

    EStateMachine(std::string &&name_, TStates&&... states_);

    void dump(std::ostream& out_, int indent_) const;

    void update(entt::registry &reg_);

    auto getInitialState();


protected:
    // Possible to use simple tuple
    std::vector<std::unique_ptr<ICompoundState<typename Dependencies::toTupleRefs>>> m_states;
    std::string m_name;
};

#endif

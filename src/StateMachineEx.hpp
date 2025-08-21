#ifndef STATE_MACHINE_EX_HPP_
#define STATE_MACHINE_EX_HPP_
#include "StateMachineEx.h"

template<typename T>
void dumpType()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

template<typename T>
constexpr auto collectAllDependencies()
{
    return typename T::Dependencies();
}

template<typename T, typename... Ty>
constexpr auto collectAllDependencies() requires (sizeof...(Ty) > 0)
{
    return typename T::Dependencies() + collectAllDependencies<Ty...>();
}

template<typename... Ts>
constexpr auto getUniqueFromDeps(TypeList<Ts...>)
{
    return uniqueTypeList<Ts...>();
}


template<typename UpdArg, typename T>
template<typename... Args>
StateAdapter<UpdArg, T>::StateAdapter(Args&&... args_) :
    T(std::forward<Args>(args_)...)
{}

template<typename UpdArg, typename T>
void StateAdapter<UpdArg, T>::dump(std::ostream& out_, int indent_) const
{
    T::dump(out_, indent_);
}

template<typename UpdArg, typename T>
void StateAdapter<UpdArg, T>::update(UpdArg arg_) const
{
    std::cout << "Adapting call from: " << std::endl;
    dumpType<UpdArg>();
    std::cout << "To: " << std::endl;
    dumpType<typename T::Dependencies>();

    auto refs = [&]<typename... Ts>(TypeList<Ts...>){
        return std::make_tuple(std::ref(std::get<Ts&>(arg_))...);
    } (typename T::Dependencies());

    dumpType<decltype(refs)>();

    T::update(refs);
}

template<typename... TComponents>
CompoundState<TComponents...>::CompoundState(TComponents&&... components_) :
    m_components(std::forward<TComponents>(components_)...)
{
}

template<typename... TComponents>
void CompoundState<TComponents...>::update(Dependencies::toTupleRefs dependencies_) const
{
    std::cout << "Real update with ";
    dumpType<decltype(dependencies_)>();
    
    std::apply([&](const auto&... comps) // Components
    {
        (([&]<typename... Ts>(TypeList<Ts...>) // Logic component's dependencies
        {
            comps.update(std::get<Ts&>(dependencies_)...);
        }(typename std::remove_reference<decltype(comps)>::type::Dependencies())), ...);
    }, m_components);
}

template<typename... TComponents>
void CompoundState<TComponents...>::dump(std::ostream& out_, int indent_) const
{
    out_ << std::string(indent_, ' ') << '+' << ttypedata<CompoundState>::name << "\n";

    ([&]
    {
        out_ << std::string(indent_ + 1, ' ') << '>' << typedata<TComponents>::name << "\n";

    } (), ...);
}


template<typename... TStates>
EStateMachine<TStates...>::Iterator::Iterator(const EStateMachine<TStates...> &machine_) :
    m_currentState(machine_.getInitialState())
{}

template<typename... TStates>
EStateMachine<TStates...>::EStateMachine(std::string &&name_, TStates&&... states_) :
    m_name(name_)
{
    ([&]
    {
        m_states.emplace_back(std::make_unique<StateAdapter<typename Dependencies::toTupleRefs, TStates>>(std::forward<TStates>(states_)));
    } (), ...);
}

template<typename... TStates>
void EStateMachine<TStates...>::dump(std::ostream& out_, int indent_) const
{
    out_ << std::string(indent_, ' ') << m_name << " => ";
    Dependencies::dump(out_);
    out_ << "\n";

    for (const auto &state : m_states)
    {
        state->dump(out_, indent_ + 2);
    }
}

template<typename... TStates>
void EStateMachine<TStates...>::update(Iterator &it_, EntityAnywhere owner_) const
{
    assert(it_.m_currentState);

    auto refs = [&owner_]<typename... TDependency>(TypeList<TDependency...>){
        return owner_.reg->get<TDependency...>(owner_.idx);
    } (Dependencies());

    std::cout << "Updating from: ";
    dumpType<decltype(refs)>();

    it_.m_currentState->update(refs);
}

template<typename... TStates>
auto EStateMachine<TStates...>::getInitialState() const
{
    return m_states.front().get();
}

#endif

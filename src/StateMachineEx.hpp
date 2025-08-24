#ifndef STATE_MACHINE_EX_HPP_
#define STATE_MACHINE_EX_HPP_
#include "StateMachineEx.h"
#include "Core/CallChain.hpp"

template<typename T>
void dumpType()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

template<typename... TCallable>
UpdateBody<TCallable...>::UpdateBody(TCallable&&... callables_) :
    BodyType(std::forward<TCallable>(callables_)...)
{}

template<typename... TCallable>
CompoundInPipe<TCallable...>::CompoundInPipe(TCallable&&... callables_) :
    BodyType(std::forward<TCallable>(callables_)...)
{}

template<typename... TPipes>
constexpr auto formInPipeSet(TPipes&&... pipes_)
{
    return std::make_tuple(std::forward<TPipes>(pipes_)...);
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

// Take unique components, return all their dependencies (at Ty::Dependencies) in 1 list
template<typename... Ty>
constexpr auto collectAllDependencies() requires (sizeof...(Ty) == 0)
{
    return TypeList<>{};
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
void StateAdapter<UpdArg, T>::dump(std::ostream&, int) const
{
    //T::dump(out_, indent_);
}

template<typename UpdArg, typename T>
void StateAdapter<UpdArg, T>::update(UpdArg arg_)
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

template<typename TUpdateBody, typename TInPipes>
CompoundState<TUpdateBody, TInPipes>::CompoundState(TUpdateBody &&updateBody_, TInPipes &&inPipes_) :
    m_updateBody(std::move(updateBody_)),
    m_inPipes(std::move(inPipes_))
{
}

template<typename TUpdateBody, typename TInPipes>
void CompoundState<TUpdateBody, TInPipes>::update(Dependencies::toTupleRefs dependencies_)
{
    std::cout << "Real update with ";
    dumpType<decltype(dependencies_)>();
    
    m_updateBody(dependencies_);
}

template<typename TUpdateBody, typename TInPipes>
void CompoundState<TUpdateBody, TInPipes>::dump(std::ostream&, int) const
{
    // No dump yet
}


template<typename... TStates>
EStateMachine<TStates...>::Iterator::Iterator(EStateMachine<TStates...> &machine_) :
    m_currentState(machine_.getInitialState())
{}

template<typename... TStates>
EStateMachine<TStates...>::EStateMachine(std::string &&name_, TStates&&... states_) :
    m_name(std::move(name_))
{
    ([&]
    {
        m_states.emplace_back(std::make_unique<StateAdapter<typename Dependencies::toTupleRefs, TStates>>(std::forward<TStates>(states_)));
    } (), ...);
}

template<typename... TStates>
void EStateMachine<TStates...>::dump(std::ostream&, int) const
{
    /*out_ << std::string(indent_, ' ') << m_name << " => ";
    Dependencies::dump(out_);
    out_ << "\n";

    for (const auto &state : m_states)
    {
        state->dump(out_, indent_ + 2);
    }*/
}

template<typename... TStates>
void EStateMachine<TStates...>::update(Iterator &it_, EntityAnywhere owner_)
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
auto EStateMachine<TStates...>::getInitialState()
{
    return m_states.front().get();
}

#endif

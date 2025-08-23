#ifndef CALL_CHAIN_HPP_
#define CALL_CHAIN_HPP_
#include "CallChain.h"
#include "TypeList.hpp"
#include <iostream>

namespace callable {

template<typename... TCallable>
Chain<TCallable...>::Chain(TCallable&&... callables_) :
    m_callables(std::forward<TCallable>(callables_)...)
{}

template<typename... TCallable>
template<typename Arg>
constexpr auto Chain<TCallable...>::operator()(const Arg& arg_)
{
    return callRecursive(arg_);
}

template<typename... TCallable>
template<size_t idx>
constexpr auto Chain<TCallable...>::callRecursive(const auto &default_)
{
    if constexpr (idx == sizeof...(TCallable) - 1)
        return std::get<sizeof...(TCallable) - idx - 1>(m_callables)(default_);
    else
        return std::get<sizeof...(TCallable) - idx - 1>(m_callables)(callRecursive<idx + 1>(default_));
}

template<typename... TCallable>
EachResolved<TCallable...>::EachResolved(TCallable&&... callables_) :
    m_callables(std::forward<TCallable>(callables_)...)
{}

template<typename... TCallable>
template<typename... Args>
constexpr void EachResolved<TCallable...>::operator()(std::tuple<Args...> args_)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::apply([&](const auto&... callable_) {
        (([&]<typename... Ts>(TypeList<Ts...>) // Logic component's dependencies
        {
            callable_.update(std::get<Ts&>(args_)...);
        }(typename std::remove_reference<decltype(callable_)>::type::Dependencies())), ...);
    }, m_callables);
}

}

#endif

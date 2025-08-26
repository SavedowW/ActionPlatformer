#ifndef CALL_CHAIN_H_
#define CALL_CHAIN_H_
#include "TypeList.hpp"

// Various ways to pack several callables together
namespace callable {

// Call first, pass result to second, to third... return final result
//
// `Chain(F1, F2, F3) chain;`
// `chain();`
// Is an equivalent of:
// `F3(F2(F1))();`
template<typename... TCallable>
class Chain
{
public:
    using Dependencies = getUniqueDependencies<TCallable...>;

    Chain(TCallable&&... callables_);

    template<typename Arg, typename... Args>
    constexpr auto operator()(const Arg& arg_, Args&... args_);

private:
    template<size_t idx = 0, typename... Args>
    constexpr auto callRecursive(const auto &default_, Args&... args_);

    std::tuple<TCallable...> m_callables;

};

// Call each in a sequence while resolving dependencies
//
// `Chain(F1, F2, F3) chain;`
// `chain(std::make_tuple(arg1, arg2, arg3));`
// Is an equivalent of:
// `F1(arg1, arg2); F2(arg3); F3(arg1, arg2, arg3);`
// Where required types are specified in Fx::Dependencies
template<typename... TCallable>
class EachResolved
{
public:
    using Dependencies = getUniqueDependencies<TCallable...>;

    EachResolved(TCallable&&... callables_);

    template<typename... Args>
    constexpr void operator()(std::tuple<Args...> args_);

private:
    std::tuple<TCallable...> m_callables;
};

}

#endif

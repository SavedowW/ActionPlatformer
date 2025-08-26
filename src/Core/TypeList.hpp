#ifndef TYPELIST_HPP_
#define TYPELIST_HPP_
#include "StaticMapping.hpp"

template<typename... Args>
struct TypeList
{
    template<typename T>
    constexpr static bool contains()
    {
        return (std::is_same_v<T, Args> || ...);
    }

    template<typename... Ts>
    constexpr auto operator+(TypeList<Ts...>)
    {
        return TypeList<Args..., Ts...>();
    }

    static void dump(std::ostream& out_)
    {
        ([&]
        {
            out_ << typedata<Args>::name << ", ";
        } (), ...);
    }

    using toTuple = std::tuple<Args...>;
    using toTupleRefs = std::tuple<Args&...>;
};

template <typename T, typename... Ts>
struct unique : std::type_identity<T> {};

template <typename... Ts, typename U, typename... Us>
struct unique<TypeList<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...)
                       , unique<TypeList<Ts...>, Us...>
                       , unique<TypeList<Ts..., U>, Us...>> {};

template <typename... Ts>
using uniqueTypeList = typename unique<TypeList<>, Ts...>::type;


// Same as below, but for a single type
template<typename T>
constexpr auto collectAllDependencies()
{
    return typename T::Dependencies();
}

// Take unique components, return all their dependencies (at Ty::Dependencies) in 1 list
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

// Take type list, return type list with only unique types
template<typename... Ts>
constexpr auto getUniqueFromDeps(TypeList<Ts...>)
{
    return uniqueTypeList<Ts...>();
}

// Take list of types, get only unique dependencies (at Ts::Dependencies)
template<typename... Ts>
using getUniqueDependencies = decltype(getUniqueFromDeps(collectAllDependencies<Ts...>()));

#endif

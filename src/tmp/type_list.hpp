#pragma once

#include <type_traits>
#include <utility>

namespace TMP
{

template<class... Args_t>
struct TypeList_t {  };

template<class...> struct TypeListExtractor_t;

template<template<class...> class TList_t, class... Args_t>
struct TypeListExtractor_t<TList_t<Args_t...>>
{
    template<class Functor_t, class... FArgs_t>
    using Functor_Return_t = decltype(
            std::declval<Functor_t>().template
                operator()<Args_t...>(std::declval<FArgs_t>()...));

    template<class Functor_t, class... FArgs_t>
    constexpr static
    Functor_Return_t<Functor_t, FArgs_t...>
    invoke_functor(FArgs_t&&... args)
    {
        return Functor_t{}.template
            operator()<Args_t...>(std::forward<FArgs_t>(args)...);
    }
};

} // namespace MPL

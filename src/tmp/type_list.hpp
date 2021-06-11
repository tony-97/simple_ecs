#pragma once

#include <type_traits>
#include <utility>

namespace TMP
{

template<class... Args_t>
struct TypeList_t {  };

template<class FirstTypeList_t, class SecondTypeList_t>
struct TypeListCatIMPL_t;

template<template <class...> class FirstTypeList_t, class... FirstTypes_t,
         template <class...> class SecondTypeList_t, class... SecondTypes_t>
struct TypeListCatIMPL_t<FirstTypeList_t<FirstTypes_t...>,
                     SecondTypeList_t<SecondTypes_t...>>
{
    using type = TypeList_t<FirstTypes_t..., SecondTypes_t...>;
};

template<class... TLists_t>
struct TypeListCat_t;

template<template <class...> class TList_t, class... Types_t>
struct TypeListCat_t<TList_t<Types_t...>>
{
    using type = TList_t<Types_t...>;
};

template<class First_t, class Second_t, class... Rest_t>
struct TypeListCat_t : TypeListCat_t<typename TypeListCatIMPL_t<First_t, Second_t>::type, Rest_t...>
{
    
};

template<class...> struct TypeListExtractor_t;

template<template<class...> class TList_t, class... Args_t>
struct TypeListExtractor_t<TList_t<Args_t...>>
{
    template<class Functor_t, class... FArgs_t>
    using Functor_Return_t = decltype(
            std::declval<Functor_t>().template
                operator()<Args_t...>(std::declval<FArgs_t>()...));

    template<class Functor_t, class... FArgs_t>
    constexpr static auto
    invoke_functor(FArgs_t&&... args)
    -> Functor_Return_t<Functor_t, FArgs_t...>
    {
        return Functor_t{}.template
            operator()<Args_t...>(std::forward<FArgs_t>(args)...);
    }
};

//TODO: Understand this

template<typename T>
struct TypeIdentity_t
{
  using type = T;
};

template <typename T, typename... Ts>
struct UniqueTypesContainerIMPL : public TypeIdentity_t<T> { };

template <template<class...> class TypesContainer_t,
          typename... Ts,
          typename U,
          typename... Us>
struct UniqueTypesContainerIMPL<TypesContainer_t<Ts...>, U, Us...>
    : std::conditional_t<
                         (std::is_same_v<U, Ts> || ...)
                         , UniqueTypesContainerIMPL<TypesContainer_t<Ts...>,
                                                    Us...>
                         , UniqueTypesContainerIMPL<TypesContainer_t<Ts..., U>,
                                                    Us...>
                        >
{ };

template <class TypesContainer_t>
struct UniqueTypesContainer;

template <template<class...>class TypesContainer_t, typename... Ts>
struct UniqueTypesContainer<TypesContainer_t<Ts...>>
: public UniqueTypesContainerIMPL<TypesContainer_t<>, Ts...>
{ };

template <class TypesContainer_t>
using UniqueTypesContainer_t =
typename UniqueTypesContainer<TypesContainer_t>::type;

} // namespace MPL

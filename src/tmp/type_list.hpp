#pragma once

#include <type_traits>
#include <utility>

namespace TMP
{

template<class... Args_t>
struct TypeList_t {  };

///////////////////////////////////////////////////////////////////////////////
// Type at index
///////////////////////////////////////////////////////////////////////////////

template<std::size_t I, class Sequence_t>
struct TypeAt;

template<std::size_t I,
         template<class...> class Sequence_t,
         class Head_t,
         class... Tail_t>
struct TypeAt<I, Sequence_t<Head_t, Tail_t...>>
: TypeAt<I - 1, class Sequence_t<Tail_t...>>
{  };

template<template<class...> class Sequence_t,
         class Head_t,
         class... Tail_t>
struct TypeAt<0, Sequence_t<Head_t, Tail_t...>>
{
    using type = Head_t;
};

template<std::size_t I, class Sequence_t>
using TypeAt_t = typename TypeAt<I, Sequence_t>::type;

///////////////////////////////////////////////////////////////////////////////
// Size
///////////////////////////////////////////////////////////////////////////////

template<class Sequence_t>
struct Size;

template<template <class...> class Sequence_t, class... Types_t>
struct Size<Sequence_t<Types_t...>>
{
    using size = std::integral_constant<std::size_t, sizeof...(Types_t)>;
};

template<class Sequence_t>
constexpr inline std::size_t Size_v = Size<Sequence_t>::size::value;

///////////////////////////////////////////////////////////////////////////////
// Concatenate Type Lists
///////////////////////////////////////////////////////////////////////////////

template<class FirstTypeList_t, class SecondTypeList_t>
struct TypeListCatIMPL;

template<template <class...> class FirstTypeList_t, class... FirstTypes_t,
         template <class...> class SecondTypeList_t, class... SecondTypes_t>
struct TypeListCatIMPL<FirstTypeList_t<FirstTypes_t...>,
                       SecondTypeList_t<SecondTypes_t...>>
{
    using type = TypeList_t<FirstTypes_t..., SecondTypes_t...>;
};

template<class FirstTList_t, class SecondTList_t>
using TypeListCatIMPL_t = typename TypeListCatIMPL<FirstTList_t,
                                                   SecondTList_t>::type;

template<class FirstTList_t, class SecondTList_t, class... RestTypes_t>
struct TypeListCat_t;

template<class FirstTList_t, class SecondTList_t>
struct TypeListCat_t<FirstTList_t, SecondTList_t>
{
    using type = TypeListCatIMPL_t<FirstTList_t, SecondTList_t>;
};

template<class First_t, class Second_t, class... Rest_t>
struct TypeListCat_t
: TypeListCat_t<TypeListCatIMPL_t<First_t, Second_t>, Rest_t...>
{  };

///////////////////////////////////////////////////////////////////////////////
// Pass types as template argument
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
// Type list with unique types
///////////////////////////////////////////////////////////////////////////////

//TODO: Understand this

template<typename T>
struct TypeIdentity
{
  using type = T;
};

template <typename T, typename... Ts>
struct UniqueTypesContainerIMPL : TypeIdentity<T> { };

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

///////////////////////////////////////////////////////////////////////////////
// Ignore order compare type list 
///////////////////////////////////////////////////////////////////////////////

template <typename T, typename Tuple>
struct TypeCounter;

template <typename T, template<class...> class Seq_t, typename ... Ts>
struct TypeCounter<T, Seq_t<Ts...>>
: std::integral_constant<std::size_t, (std::is_same_v<T, Ts> + ...)>
{  };

template <typename T, typename Sequence_t>
inline constexpr std::size_t TypeCounter_v = TypeCounter<T, Sequence_t>::value;

template <typename FirstSeq_t, typename SecondSeq_t, std::size_t... Is>
constexpr bool IEqualTypes(std::index_sequence<Is...>)
{
    return (...
            && (   TypeCounter_v<TypeAt<Is, FirstSeq_t>, FirstSeq_t>
                == TypeCounter_v<TypeAt<Is, FirstSeq_t>, SecondSeq_t>)
           );
}

template <typename FirstSeq_t, typename SecondSeq_t>
constexpr bool IEqualTypes()
{
    constexpr auto s1 = Size_v<FirstSeq_t>;
    constexpr auto s2 = Size_v<SecondSeq_t>;

    return s1 == s2
           && IEqualTypes<FirstSeq_t,
                          SecondSeq_t>(std::make_index_sequence<s1>());
}

} // namespace TMP

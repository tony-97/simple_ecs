#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ECS
{

// assert with a mensage from cppreference
#define assert_msg(exp, msg) assert(((void)(msg), (exp)))

///////////////////////////////////////////////////////////////////////////////
// contains
///////////////////////////////////////////////////////////////////////////////
template<typename T, typename ...Ts>
struct IsOneOf
{
    constexpr static inline bool value
    {
        std::disjunction_v<std::is_same<T, Ts>...>
    };
};

///////////////////////////////////////////////////////////////////////////////
// SameAsConstMemFunc
///////////////////////////////////////////////////////////////////////////////

// https://stackoverflow.com/a/16780327
template <typename T> struct NonConst_t
{
    typedef T type;
};

template <typename T> struct NonConst_t<T const>
{
    typedef T type;
}; //by value

template <typename T> struct NonConst_t<T const&>
{
    typedef T& type;
}; //by reference

template <typename T> struct NonConst_t<T const*>
{
    typedef T* type;
}; //by pointer

template <typename T> struct NonConst_t<T const&&>
{
    typedef T&& type;
}; //by rvalue-reference

template<typename ConstReturn_t,
         class Obj_t,
         typename ...MemFnArgs_t,
         typename ...Args_t>
typename NonConst_t<ConstReturn_t>::type
SameAsConstMemFunc(
    Obj_t const* this_ptr,
    ConstReturn_t (Obj_t::* memFun)(MemFnArgs_t...) const,
    Args_t&& ...args)
{
    return const_cast<typename NonConst_t<ConstReturn_t>::type>
        ((this_ptr->*memFun)(std::forward<Args_t>(args)...));
}

///////////////////////////////////////////////////////////////////////////////
// Uncopyable_t
///////////////////////////////////////////////////////////////////////////////
struct Uncopyable_t
{
    Uncopyable_t() = default;

    Uncopyable_t(Uncopyable_t&&)                 = delete;
    Uncopyable_t(const Uncopyable_t&)            = delete;
    Uncopyable_t& operator=(Uncopyable_t&&)      = delete;
    Uncopyable_t& operator=(const Uncopyable_t&) = delete;
};

///////////////////////////////////////////////////////////////////////////////
// AreUnique_t
///////////////////////////////////////////////////////////////////////////////
template<class ...Args_t>
struct AreUnique_t;

template<class T0, class T1, class ...Tn>
struct AreUnique_t<T0, T1, Tn...>
{
    constexpr static inline bool value
    {
        !std::disjunction_v<std::is_same<T0, T1>,
                            std::is_same<T0, Tn>...,
                            std::is_same<T1, Tn>...>
    };
};

template<class T>
struct AreUnique_t<T>
{
    constexpr static inline bool value { true };
};

///////////////////////////////////////////////////////////////////////////////
// IndexOfElement_t
///////////////////////////////////////////////////////////////////////////////

//https://stackoverflow.com/questions/18063451/get-index-of-a-tuple-elements-type/60868425

template<class Element_t, class TElements_t>
struct IndexOfElement_t
{

private:
    template<std::size_t I, class T, class TElemns_t>
    static constexpr auto GetIndex() -> std::size_t
    {
        constexpr bool isOutOfRange = I >= std::tuple_size_v<TElemns_t>; 

        static_assert(!isOutOfRange, "The element does not exist.");

        if constexpr (!isOutOfRange) {
            using element_tp = std::tuple_element_t<I, TElemns_t>;
            if constexpr (std::is_same_v<T, element_tp>) {
                return I;
            } else {
                return GetIndex<I + 1, T, TElemns_t>();
            }
        }
    }

public:
    constexpr static inline std::size_t value
    {
        GetIndex<0, Element_t, TElements_t>()
    };

};

} // namespace ECS

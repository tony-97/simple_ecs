#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <optional>

namespace ECS
{

using EntityID_t        = std::size_t;
using ComponentTypeID_t = std::size_t;

template<class Component_t>
struct ComponentID_t
{
    using Component_type = Component_t;
    std::size_t ID;
};

template<typename T>
using Storage_t = std::vector<T>;

template<typename T>
using RemovePCR = std::remove_pointer_t<std::decay_t<T>>;

template<class T>
using Owned_t    = std::unique_ptr<T>;

template<class T>
using Optional_t = std::optional<T>;

template<class T>
using Nullable_t = std::optional<std::reference_wrapper<T>>;

template<class ...Ts>
using Elements_t = std::tuple<Ts...>;

template<class T1, class T2>
using Combine_t = std::pair<T1, T2>;

} // namespace ECS

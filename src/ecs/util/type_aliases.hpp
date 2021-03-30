#pragma once

#include <tuple>
#include <vector>
#include <memory>
#include <optional>

#include "increment.hpp"
#include "identificable.hpp"

namespace ECS
{

using EntityID_t        = std::size_t;
using ComponentID_t     = std::size_t;
using ComponentTypeID_t = std::size_t;

template<typename T>
using Storage_t         = std::vector<T>;

template<typename MAIN_CMP_t, typename ...EXTRA_CMPS_t>
using SystemUpdate = auto (*)(MAIN_CMP_t&& main_cmp,
                              EXTRA_CMPS_t&&... extra_cmps) -> void;

template<typename T>
using RemovePCR = typename std::remove_const
                        <typename std::remove_reference
                            <typename std::remove_pointer<T>::type>
                                                                ::type>
                                                                    ::type;

template<typename T, typename ID_t>
using IndexableInstance_t = IdentificableForEachInstance_t<
                                                           T,
                                                           ID_t,
                                                           PostIncrement<ID_t>>;

template<class T>
using Owned_t = std::unique_ptr<T>;

template<class T>
using Optional_t = std::optional<T>;

template<class T>
using Nullable_t = std::optional<std::reference_wrapper<T>>;

template<class ...Ts>
using Elements_t = std::tuple<Ts...>;

} // namespace ECS

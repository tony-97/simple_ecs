#pragma once

#include <tuple>

#include <ecs/util/type_aliases.hpp>
#include <ecs/util/helpers.hpp>

#include <tmp/type_list.hpp>
#include <utility>

namespace ECS
{
template<class Signature_t>
struct ExtractComponentsFromSystems;

template<template<class...> class Signature_t, class... Systems_t>
struct ExtractComponentsFromSystems<Signature_t<Systems_t...>>
{
    using type = TMP::UniqueTypeList_t<
                 TMP::TypeListCat_t<typename Systems_t::SystemSignature_t...>>;
};

template<class Signature_t>
using ExtractComponentsFromSystems_t =
typename ExtractComponentsFromSystems<Signature_t>::type;

template<class EntitySignature_t>
struct ElementsOfComponentIDs;

template<template<class...> class EntitySignature_t, class... Components_t>
struct ElementsOfComponentIDs<EntitySignature_t<Components_t...>>
{
    using type = std::tuple<ComponentID_t<Components_t>...>;
};

template<class EntitySignature_t>
using ElementsOfComponentIDs_t =
typename ElementsOfComponentIDs<EntitySignature_t>::type;

template<class Components_t>
struct EntityBase_t
{
    using TableOfComponentIDs_t = ElementsOfComponentIDs_t<Components_t>;
protected:

    template<class... ComponentIDs_t>
    explicit constexpr EntityBase_t(ComponentIDs_t&&... cmp_ids)
    : mCompIDs{ std::forward<ComponentIDs_t>(cmp_ids)... } {  }

    template<class ReqCmp_t>
    auto GetRequiredComponentID() const
    -> ComponentID_t<ReqCmp_t>
    {
        return std::get<ComponentID_t<ReqCmp_t>>(mCompIDs);
    }

    template<class ReqCmp_t>
    auto GetRequiredComponentID(ComponentTypeID_t cmp_tp_id)
    -> ComponentID_t<ReqCmp_t>
    {
        return const_cast<const EntityBase_t*>
               (this)->GetRequiredComponentID(cmp_tp_id);
    }

    template<class ReqCmpID_t>
    auto UpdateRequieredComponentID(ReqCmpID_t cmp_id)
    -> void
    {
        std::get<ReqCmpID_t>(mCompIDs) = std::move(cmp_id);
    }

    template<class Callable_t>
    constexpr auto
    ForEachComponentID(Callable_t&& callable)
    {
        ForEachElement(mCompIDs, std::forward(callable));
    }

private:
    TableOfComponentIDs_t mCompIDs {  };
};

template<class EntMan_t, class Systems_t>
struct Entity_t : EntityBase_t<ExtractComponentsFromSystems_t<Systems_t>>,
                  Uncopyable_t
{
    friend EntMan_t;
    using Base_t = EntityBase_t<ExtractComponentsFromSystems_t<Systems_t>>;

private:

    template<class... ComponentIDs_t>
    explicit constexpr Entity_t(EntityID_t eid, ComponentIDs_t&&... cmp_ids)
    : Base_t{ std::forward<ComponentIDs_t>(cmp_ids)... }, mID{ eid } {  }

    Entity_t& operator=(const Entity_t& other)
    {
        Base_t::operator=(std::move(other));
        return *this;
    }

    std::size_t mID{  };
};

} // namespace ECS

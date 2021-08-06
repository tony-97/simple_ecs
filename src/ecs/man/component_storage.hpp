#pragma once

#include <variant>
#include <array>
#include <utility>

#include <ecs/util/type_aliases.hpp>
#include <ecs/util/helpers.hpp>

#include <tmp/type_list.hpp>

namespace ECS
{

template<class ComponentTypes> class ComponentStorage_t;

template<template<class...> class ComponentTypes_t,  class... Components_t>
class ComponentStorage_t<ComponentTypes_t<Components_t...>> final : Uncopyable_t
{

public:

    template<class Component_t>
    struct InternalComponent_t final
    {
        using EntityID_type  = EntityID_t;
        using Component_type = Component_t;

        template<class EID_t, class... Args_t>
        constexpr explicit
        InternalComponent_t(EID_t&& eid, Args_t&&... args)
        : EntityID{ std::forward<EID_t>(eid) },
          Self{ std::forward<Args_t>(args)... } {  }

        template<class EID_t, class TupleArgs_t>
        constexpr
        InternalComponent_t(std::piecewise_construct_t,
                            EID_t&& eid,
                            TupleArgs_t&& args)
        : InternalComponent_t(std::forward<EID_t>(eid),
                              std::forward<TupleArgs_t>(args),
                              std::make_index_sequence<
                                std::tuple_size_v<
                                    std::remove_reference_t<TupleArgs_t>>>{})
        {  }

    private:
        template<class EID_t, class TupleArgs_t, std::size_t... I>
        constexpr
        InternalComponent_t(EID_t&& eid,
                            TupleArgs_t&& args,
                            std::index_sequence<I...>)
        : EntityID { std::forward<EID_t>(eid) },
          Self{ std::get<I>(std::forward<TupleArgs_t>(args))... }
        {  }

    public:
        constexpr auto GetEntityID()       -> EntityID_t { return EntityID; }
        constexpr auto GetEntityID() const -> EntityID_t { return EntityID; }

        EntityID_t EntityID {  };
        Component_t Self {  };
    };

    template<class T>
    using VecComponent_t = Storage_t<InternalComponent_t<T>>;

    using TableOfComponents_t =
        std::array<std::variant<VecComponent_t<Components_t>...>,
                   sizeof...(Components_t)>;

    constexpr explicit ComponentStorage_t()
    {
        CheckIfComponentsAreUnique();
    }

    template<typename ReqCmp_t, typename ...Args_t>
    constexpr auto
    CreateRequieredComponentFromArgs(Args_t&& ...args)
    -> Combine_t<ComponentID_t, ReqCmp_t&>
    {
        auto& vec_cmps { GetRequieredComponentStorage<ReqCmp_t>() };
        auto cmp_id { vec_cmps.size() };
        auto& cmp { vec_cmps.emplace_back(std::forward<Args_t>(args)...) };
        return { cmp_id, cmp.Self };
    }

    template<typename ReqCmp_t, typename ...Args_t>
    constexpr auto
    CreateRequieredComponent(EntityID_t eid, Args_t&& ...args)
    -> Combine_t<ComponentID_t, ReqCmp_t&>
    {
        return
        CreateRequieredComponentFromArgs<ReqCmp_t>
        (
         eid, std::forward<Args_t>(args)...
        );
    }

    //TODO: make one createrequieredcomponent(s)
    template<typename... ReqCmps_t, typename... TupleArgs_t>
    constexpr auto
    CreateRequieredComponents(EntityID_t eid, TupleArgs_t&&... args)
    -> Elements_t<Combine_t<ComponentID_t, ReqCmps_t&>...>
    {
        if constexpr (sizeof...(TupleArgs_t) < sizeof...(ReqCmps_t))
        {
            constexpr auto diff {sizeof...(ReqCmps_t) - sizeof...(TupleArgs_t)};
            constexpr auto empty_args
            {
                MakeEmptyArgs(std::make_index_sequence<diff>{})
            };
            constexpr auto tuple_args
            {
                MakeForwadTuple(std::forward<TupleArgs_t>(args)...)
            };
            auto eid_arg { MakeForwadTuple(eid) };
            auto self_arg{ MakeForwadTuple(this) };
            constexpr auto cat_args { std::tuple_cat(self_arg, eid_arg, tuple_args, empty_args) };

            return std::apply(&ComponentStorage_t::CreateRequieredComponents<ReqCmps_t...>, cat_args);
        } else {
            return
            {
                CreateRequieredComponentFromArgs<ReqCmps_t>
                (
                 std::piecewise_construct,
                 eid,
                 std::forward<TupleArgs_t>(args)
                )...
            };
        }
    }

    constexpr auto
    SetComponentEntityID(ComponentTypeID_t cmp_tp_id,
                         ComponentID_t cmp_id,
                         EntityID_t eid)
    -> void
    {
        auto update_entity_id
        {
            [&cmp_id, &eid](auto& cmps)
            {
                
            }
        };

        auto& vec_variant { mComponentVectors[cmp_tp_id] };
        return std::visit(update_entity_id, vec_variant);
        //mVecs[cmp_tp_id]->UpdateComponentEntityID(cmp_id, eid);
    }

    constexpr auto
    RemoveComponentByTypeIDAndID(ComponentTypeID_t cmp_tp_id,
                                 ComponentID_t cmp_id)
    -> EntityID_t
    {
        auto remove_component_by_index
        {
            [&cmp_id](auto& cmps)
            {
                
            }
        };

        auto& vec_variant { mComponentVectors[cmp_tp_id] };
        return std::visit(remove_component_by_index, vec_variant);
        //return mVecs[cmp_tp_id]->RemoveComponentByIndex(cmp_id);
    }

    template<typename ReqCmp_t>
    static constexpr auto
    GetRequiredComponentTypeIndex()
    -> ComponentTypeID_t
    {
        constexpr auto index
        {
            TMP::IndexOf_v<RemovePCR<ReqCmp_t>,
                           TMP::TypeList_t<Components_t...>>
        };
        return static_cast<ComponentTypeID_t>(index);
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentStorage() const
    -> const VecComponent_t<RemovePCR<ReqCmp_t>>&
    {
        CheckIfComponentIsInThisInstance<RemovePCR<ReqCmp_t>>();

        constexpr auto index { GetRequiredComponentIndexID<ReqCmp_t>() };
        auto& vec_variant
        {
            mComponentVectors[index]
        };

        return std::get<VecComponent_t<RemovePCR<ReqCmp_t>>>(vec_variant);
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentStorage()
    -> VecComponent_t<RemovePCR<ReqCmp_t>>&
    {
        return SameAsConstMemFunc
               (
                this,
                &ComponentStorage_t::template
                GetRequieredComponentStorage<ReqCmp_t>
               );
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentByID(ComponentID_t cmp_id) const
    -> const ReqCmp_t&
    {
        auto& cmps { GetRequieredComponentStorage<ReqCmp_t>() };

        return cmps[cmp_id].Self;
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentByID(ComponentID_t cmp_id)
    -> ReqCmp_t&
    {
        return SameAsConstMemFunc
               (
                this,
                &ComponentStorage_t::template
                GetRequieredComponentByID<ReqCmp_t>,
                cmp_id
               );
    }

private:

    template<typename ReqCmp_t>
    static constexpr auto CheckIfComponentIsInThisInstance() -> void
    {
        static_assert(IsOneOf<ReqCmp_t, Components_t...>::value,
                      "The requiered component type does not"
                      " exist in this ComponentStorage_t");
    }

    static constexpr auto CheckIfComponentsAreUnique() -> void
    {
        static_assert(AreUnique_t<Components_t...>::value,
                      "Components need to be unique");
    }

    TableOfComponents_t mComponentVectors {  };
};

} // namespace ECS

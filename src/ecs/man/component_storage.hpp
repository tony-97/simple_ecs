#pragma once

#include <tuple>
#include <type_traits>

#include <ecs/util/type_aliases.hpp>
#include <ecs/util/helpers.hpp>

namespace ECS
{

template<class... Components_t>
class ComponentStorage_t final : Uncopyable_t
{
public:

    template<class Component_t>
    struct InternalComponent_t final
    {
        friend ComponentStorage_t<Components_t...>;

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

        template<class EID_t, class TupleArgs_t, std::size_t... I>
        constexpr
        InternalComponent_t(EID_t&& eid,
                            TupleArgs_t&& args,
                            std::index_sequence<I...>)
        : EntityID { std::forward<EID_t>(eid) },
          Self{ std::get<I>(std::forward<TupleArgs_t>(args))... }
        {  }

        constexpr auto GetEntityID()       -> EntityID_t { return EntityID; }
        constexpr auto GetEntityID() const -> EntityID_t { return EntityID; }

        constexpr auto SetEntityID(EntityID_t eid) -> void
        {
            EntityID = eid;
        }

    private:
        EntityID_t EntityID {  };
    public:
        Component_t Self {  };
    };

    template<class T>
    using VecComponent_t = Storage_t<InternalComponent_t<T>>;

    using TableOfComponents_t = Elements_t<VecComponent_t<Components_t>...>;

    constexpr explicit ComponentStorage_t()
    {
        CheckIfComponentsAreUnique();
    }

    template<typename ReqCmp_t, typename ...Args_t>
    constexpr auto
    CreateRequieredComponentFromArgs(Args_t&& ...args)
    -> Combine_t<ComponentID_t<RemovePCR<ReqCmp_t>>, ReqCmp_t&>
    {
        auto& vec_cmps { GetRequieredComponentStorage<ReqCmp_t>() };
        auto cmp_id { vec_cmps.size() };
        auto& cmp { vec_cmps.emplace_back(std::forward<Args_t>(args)...) };
        return { cmp_id, cmp.Self };
    }

    template<typename ReqCmp_t, typename ...Args_t>
    constexpr auto
    CreateRequieredComponent(EntityID_t eid, Args_t&& ...args)
    -> Combine_t<ComponentID_t<RemovePCR<ReqCmp_t>>, ReqCmp_t&>
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
    -> Elements_t<Combine_t<RemovePCR<ReqCmps_t>, ReqCmps_t&>...>
    {
        if constexpr (sizeof...(TupleArgs_t) < sizeof...(ReqCmps_t))
        {
            constexpr auto diff {sizeof...(ReqCmps_t) - sizeof...(TupleArgs_t)};
            constexpr auto empty_args { MakeEmptyArgs(std::make_index_sequence<diff>{}) };
            constexpr auto tuple_args { MakeArgs(std::forward<TupleArgs_t>(args)...) };
            auto eid_arg { MakeArgs(eid) };
            constexpr auto cat_args { std::tuple_cat(eid_arg, tuple_args, empty_args) };

            return std::apply(&ComponentStorage_t::CreateRequieredComponents<ReqCmps_t...>, this, cat_args);
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

    template<class ReqCmpID_t>
    constexpr auto
    SetComponentEntityID(ReqCmpID_t&& cmp_id, EntityID_t eid)
                         
    -> void
    {
        auto& int_cmp { GetInternalComponentByID<ReqCmpID_t>(cmp_id) };
        int_cmp.SetEntityID(eid);
    }

    template<class ReqCmpID_t>
    constexpr auto
    RemoveRequieredComponent(ReqCmpID_t&& cmp_id)
    -> EntityID_t
    {
        using Component_t = typename RemovePCR<ReqCmpID_t>::Component_type;
        auto& vec_cmps { GetRequieredComponentStorage<Component_t>() };
        auto& last_cmp { vec_cmps.back() };
        auto& rem_cmp  { vec_cmps[cmp_id] };
        auto eid { last_cmp.GetEntityID() };
        if constexpr (std::is_move_assignable_v<Component_t>) {
            rem_cmp = std::move(last_cmp);
        } else {
            rem_cmp = last_cmp;
        }
        vec_cmps.pop_back();

        return eid;
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentStorage() const
    -> const VecComponent_t<RemovePCR<ReqCmp_t>>&
    {
        CheckIfComponentIsInThisInstance<RemovePCR<ReqCmp_t>>();

        auto& vec_cmps
        {
            std::get<VecComponent_t<RemovePCR<ReqCmp_t>>>(mComponentVectors)
        };

        return vec_cmps;
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
    GetInternalComponentByID(ComponentID_t<RemovePCR<ReqCmp_t>> cmp_id) const
    -> const InternalComponent_t<RemovePCR<ReqCmp_t>>&
    {
        auto& cmps { GetRequieredComponentStorage<ReqCmp_t>() };

        return cmps[cmp_id];
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetInternalComponentByID(ComponentID_t<RemovePCR<ReqCmp_t>> cmp_id)
    -> const InternalComponent_t<RemovePCR<ReqCmp_t>>&
    {
        return SameAsConstMemFunc
               (
                this,
                &ComponentStorage_t::template
                GetInternalComponentByID<ReqCmp_t>,
                cmp_id
               );
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentByID(ComponentID_t<RemovePCR<ReqCmp_t>> cmp_id) const
    -> const ReqCmp_t&
    {
        return GetInternalComponentByID<ReqCmp_t>(cmp_id).Self;
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentByID(ComponentID_t<RemovePCR<ReqCmp_t>> cmp_id)
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

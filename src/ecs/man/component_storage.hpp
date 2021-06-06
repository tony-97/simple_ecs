#pragma once

#include <array>
#include <tuple>
#include <type_traits>
#include <utility>

#include <ecs/util/type_aliases.hpp>
#include <ecs/util/helpers.hpp>
#include <vector>

namespace ECS
{

struct ComponentVectorBase_t
{
    virtual ~ComponentVectorBase_t() = default;

    virtual EntityID_t
    RemoveComponentByIndex(ComponentID_t cmp_id) = 0;

    virtual void
    UpdateComponentEntityID(ComponentID_t cmp_id, EntityID_t eid) = 0;
};

template<class T>
struct VectorComponent_t final : public ComponentVectorBase_t
{
    Storage_t<T> mComponents {  };

    auto
    RemoveComponentByIndex(ComponentID_t cmp_id)
    -> EntityID_t override
    {
        auto index { static_cast<std::size_t>(cmp_id) };
        auto& last_cmp { mComponents.back() };
        auto& rem_cmp  { mComponents[index] };
        auto eid { last_cmp.GetEntityID() };

        rem_cmp = last_cmp;
        mComponents.pop_back();

        return eid;
    }

    auto
    UpdateComponentEntityID(ComponentID_t cmp_id, EntityID_t eid)
    -> void override
    {
        auto index { static_cast<std::size_t>(cmp_id) };
        mComponents[index].SetEntityID(eid);
    }
};

template<class EntMan_t> class ComponentStorage_t;

template<template<class...> class EntMan_t,  class... Components_t>
class ComponentStorage_t<EntMan_t<Components_t...>> final : Uncopyable_t
{

public:

    template<class EntID_t, class Component_t>
    struct InternalComponent_t final
    {
        friend ComponentStorage_t<EntMan_t<Components_t...>>;
        friend EntMan_t<Components_t...>;

        using EntityID_type  = EntID_t;
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

        constexpr auto GetEntityID()       -> EntID_t { return EntityID; }
        constexpr auto GetEntityID() const -> EntID_t { return EntityID; }

        constexpr auto SetEntityID(EntID_t eid) -> void
        {
            EntityID = eid;
        }

    private:
        EntID_t EntityID {  };
    public:
        Component_t Self {  };
    };

    template<class T>
    using IComponent_t = InternalComponent_t<EntityID_t, T>;

    template<class T>
    using VecComponent_t = Storage_t<IComponent_t<T>>;

    template<class T>
    using InternalVector_t = VectorComponent_t<IComponent_t<T>>; 

    using TableOfComponents_t = Elements_t<InternalVector_t<Components_t>...>;

    constexpr explicit ComponentStorage_t()
    {
        CheckIfComponentsAreUnique();
        InitVecsPtr();
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

    constexpr auto
    SetComponentEntityID(ComponentTypeID_t cmp_tp_id,
                         ComponentID_t cmp_id,
                         EntityID_t eid)
    -> void
    {
        mVecs[cmp_tp_id]->UpdateComponentEntityID(cmp_id, eid);
    }

    constexpr auto
    RemoveComponentByTypeIDAndID(ComponentTypeID_t cmp_tp_id,
                                 ComponentID_t cmp_id)
    -> EntityID_t
    {
        return mVecs[cmp_tp_id]->RemoveComponentByIndex(cmp_id);
    }

    template<typename ReqCmp_t>
    static constexpr auto
    GetRequiredComponentTypeID()
    -> ComponentTypeID_t
    {
        auto index
        {
            IndexOfElement_t<InternalVector_t<RemovePCR<ReqCmp_t>>,
                             TableOfComponents_t>::value
        };
        return static_cast<ComponentTypeID_t>(index);
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentStorage() const
    -> const VecComponent_t<RemovePCR<ReqCmp_t>>&
    {
        CheckIfComponentIsInThisInstance<RemovePCR<ReqCmp_t>>();

        auto& vec_cmps
        {
            std::get<InternalVector_t<RemovePCR<ReqCmp_t>>>(mComponentVectors)
        };

        return vec_cmps.mComponents;
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

    template<std::size_t I = 0>
    constexpr auto InitVecsPtr() -> void
    {
        if constexpr (I < sizeof...(Components_t)) {
            auto& vec_cmps { std::get<I>(mComponentVectors) };
            mVecs[I] = static_cast<ComponentVectorBase_t*>(&vec_cmps);
            InitVecsPtr<I + 1>();
        }
    } 

    TableOfComponents_t mComponentVectors {  };
    std::array<ComponentVectorBase_t*,
               sizeof...(Components_t)> mVecs {  };
};

} // namespace ECS

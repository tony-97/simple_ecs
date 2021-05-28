#pragma once

#include <tuple>
#include <utility>

#include <ecs/util/type_aliases.hpp>
#include <ecs/util/helpers.hpp>

namespace ECS
{

template<class EntMan_t> class ComponentStorage_t;

template<template<class...> class EntMan_t,  class... Components_t>
class ComponentStorage_t<EntMan_t<Components_t...>> final : Uncopyable_t
{

public:

    template<class EntID_t, class Component_t>
    struct InternalComponent_t
    {
    
        friend class ComponentStorage_t<EntMan_t<Components_t...>>;
        friend class EntMan_t<Components_t...>;

        template<class EID_t, class... Args_t>
        constexpr explicit InternalComponent_t(EID_t&& eid, Args_t&&... args)
        : EntityID{ std::forward<EID_t>(eid) },
          Self{ std::forward<Args_t>(args)... } {  }

    private:
        EntID_t EntityID {  };
    public:
        Component_t Self {  };
    };

    template<class T>
    using VecComponent_t = Storage_t<InternalComponent_t<EntityID_t, T>>;

    using TableOfComponents_t = Elements_t<VecComponent_t<Components_t>...>;

    constexpr explicit ComponentStorage_t()
    {
        CheckIfComponentsAreUnique();
    }

    template<typename ReqCmp_t, typename ...Args_t>
    constexpr
    auto CreateRequieredComponent(EntityID_t eid, Args_t&& ...args)
    -> Combine_t<ComponentID_t, ReqCmp_t&>
    {
        auto& vec_cmps { GetRequieredComponentStorage<ReqCmp_t>() };

        auto& cmp { vec_cmps.emplace_back(eid, std::forward<Args_t>(args)...) };

        return { vec_cmps.size() - 1, cmp.Self };
    }

    template<typename ReqCmp_t>
    static constexpr auto
    GetRequiredComponentTypeID() -> ComponentTypeID_t
    {
        auto index
        {
            IndexOfElement_t<VecComponent_t<ReqCmp_t>,
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

        return std::get<VecComponent_t<RemovePCR<ReqCmp_t>>>(m_Components);
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
    GetRequieredComponentByID(ComponentID_t cmp_id) -> ReqCmp_t&
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

    TableOfComponents_t m_Components {  };
};

} // namespace ECS

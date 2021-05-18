#pragma once

#include <type_traits>
#include <unordered_map>
#include <utility>

#include "component_storage.hpp"

#include <ecs/util/type_aliases.hpp>
#include <ecs/util/helpers.hpp>
#include <ecs/cmp/entity.hpp>
#include <tmp/type_list.hpp>

namespace ECS
{

struct FunctorHelperExtractor_t
{
    template<class MainComponent_t,
             class... ExtraComponents_t,
             class EntManager_t,
             class Callable_t>
    constexpr auto
    operator()(EntManager_t&& ent_man, Callable_t&& callable) -> void
    {
        auto vec_cmps { ent_man.template
                        GetRequieredComponentStorage<MainComponent_t>() };

        for (auto& cmp : vec_cmps) {
            auto& ent { ent_man.template GetEntityByComponent(cmp) };
            std::apply
            (
             callable,
             Elements_t<MainComponent_t, ExtraComponents_t...>
             {
              cmp,
              ent_man.template GetRequieredComponent<ExtraComponents_t>(ent)... 
             }
            );
        }
    }
};

template<typename CMP_t, typename ...CMPS_t>
struct EntityManager_t final
{

    using Self_t = EntityManager_t<CMP_t, CMPS_t...>;
    using OwnEntity_t = Entity_t<Self_t>;

    constexpr explicit EntityManager_t()
    {

    }

    auto CreateEntity() -> OwnEntity_t&
    {
        const auto ent_id { m_Entities.size() };
        m_Entities.push_back(OwnEntity_t{ ent_id });
        return m_Entities.back();
    }

    auto GetEntities() const -> const Storage_t<OwnEntity_t>&
    {
        return m_Entities;
    }

    auto GetEntities() -> Storage_t<OwnEntity_t>&
    {
        return SameAsConstMemFunc(this, &EntityManager_t::GetEntities);
    }

    auto GetEntityByID(EntityID_t ent_id) const -> const OwnEntity_t&
    {
        return GetEntities()[ent_id];
    }

    auto GetEntityByID(EntityID_t ent_id) -> OwnEntity_t&
    {
        return SameAsConstMemFunc(this,
                                  &EntityManager_t::GetEntityByID,
                                  ent_id);
    }

    template<typename Component_t>
    constexpr auto GetEntityByComponent(Component_t&& cmp) -> OwnEntity_t&
    {
        return GetEntityByID(cmp.GetEntityID());
    }

    template<typename REQ_CMP_t>
    constexpr auto
    GetRequieredComponentStorage() const
                                       -> const Storage_t<RemovePCR<REQ_CMP_t>>&
    {
        return m_Components.template GetRequieredComponentStorage<REQ_CMP_t>();
    }

    template<typename REQ_CMP_t>
    constexpr auto
    GetRequieredComponentStorage() -> Storage_t<RemovePCR<REQ_CMP_t>>&
    {
        return SameAsConstMemFunc
               (
                 this,
                 &EntityManager_t:: 
                 GetRequieredComponentStorage<REQ_CMP_t>
               );
    }

    template<typename REQ_CMP_t, typename ...Args>
    constexpr
    auto CreateRequieredComponent(OwnEntity_t& ent, Args&& ...args) 
                                                                   -> REQ_CMP_t&
    {
        auto& cmp
        {
            m_Components.
                    template CreateRequieredComponent<REQ_CMP_t>
                             (
                              ent.GetEntityID(),
                              std::forward<Args>(args)...
                             )
        };

        const auto cmp_tp_id
        {
            m_Components.template GetRequiredComponentTypeID<REQ_CMP_t>()
        };

        ent.AttachComponentID(cmp_tp_id, cmp.GetComponentID());

        return cmp;
    }

    template<typename OptionalCMPT_t,
             typename REQ_CMP_t = typename OptionalCMPT_t::value_type::type>
    constexpr auto
    GetRequieredComponent(const OwnEntity_t& ent) const
                                                -> const Nullable_t<const REQ_CMP_t>
    {
        const auto cmp_tp_id
        {
            m_Components.template GetRequiredComponentTypeID<REQ_CMP_t>()
        };

        const auto cmp_id
        {
            ent.FindRequiredComponentID(cmp_tp_id)
        };

        Nullable_t<const REQ_CMP_t> optional_cmp {  };
        if (cmp_id) {
            auto& cmp
            {
                m_Components.template GetRequieredComponent<REQ_CMP_t>(*cmp_id)
            };
            optional_cmp.emplace(std::ref(cmp));
        } 

        return optional_cmp;
    }

    template<typename OptionalCMPT_t,
             typename REQ_CMP_t = typename OptionalCMPT_t::value_type::type>
    constexpr auto
    GetRequieredComponent(const OwnEntity_t& ent) -> const Nullable_t<REQ_CMP_t> 
    {
        const auto cmp_tp_id
        {
            m_Components.template GetRequiredComponentTypeID<REQ_CMP_t>()
        };

        const auto cmp_id
        {
            ent.FindRequiredComponentID(cmp_tp_id)
        };

        Nullable_t<REQ_CMP_t> optional_cmp {  };
        if (cmp_id) {
            auto& cmp
            {
                m_Components.template GetRequieredComponent<REQ_CMP_t>(*cmp_id)
            };
            optional_cmp.emplace(std::ref(cmp));
        }

        return optional_cmp;
    }

    template<typename REQ_CMP_t,
             typename std::enable_if_t<IsOneOf<RemovePCR<REQ_CMP_t>,
                                       CMP_t, CMPS_t...>::value, bool> = true>
    constexpr auto
    GetRequieredComponent(const OwnEntity_t& ent) const
                                                -> const REQ_CMP_t&
    {
        const auto cmp_tp_id
        {
            m_Components.template GetRequiredComponentTypeID<REQ_CMP_t>()
        };

        const auto cmp_id
        {
            ent.GetRequiredComponentID(cmp_tp_id)
        };

        return m_Components.template GetRequieredComponent<REQ_CMP_t>(cmp_id);
    }

    template<typename REQ_CMP_t,
             typename std::enable_if_t<IsOneOf<RemovePCR<REQ_CMP_t>,
                                       CMP_t, CMPS_t...>::value, bool> = true>
    constexpr auto
    GetRequieredComponent(const OwnEntity_t& ent) -> REQ_CMP_t& 
    {
        const auto cmp_tp_id
        {
            m_Components.template GetRequiredComponentTypeID<REQ_CMP_t>()
        };

        const auto cmp_id
        {
            ent.GetRequiredComponentID(cmp_tp_id)
        };

        return m_Components.template GetRequieredComponent<REQ_CMP_t>(cmp_id);
    }

    template<typename ...REQ_CMPS_t>
    constexpr auto
    GetRequieredComponents([[maybe_unused]]const OwnEntity_t& ent) const
                                            -> const Elements_t<const REQ_CMPS_t...>
    {
        return { GetRequieredComponent<REQ_CMPS_t>(ent)... };
    }

    template<typename ...REQ_CMPS_t>
    constexpr auto
    GetRequieredComponents([[maybe_unused]]const OwnEntity_t& ent)
                                                -> Elements_t<REQ_CMPS_t...>
    {
        return SameAsConstMemFunc
               (
                this,
                &EntityManager_t::GetRequieredComponents<REQ_CMPS_t...>,
                ent
               );
    }

    template<class MAIN_CMP_t, class ...EXTRA_CMPS_t>
    constexpr auto
    DoForEachComponentType(void (*sys_upd)(MAIN_CMP_t, EXTRA_CMPS_t...)) -> void
    {
        for (auto& cmp : GetRequieredComponentStorage<MAIN_CMP_t>()) {
            auto& ent { GetEntityByID(cmp.GetEntityID()) };
            std::apply
            (
             sys_upd,
             Elements_t<MAIN_CMP_t, EXTRA_CMPS_t...>
             {
              cmp,
              GetRequieredComponent<EXTRA_CMPS_t>(ent)... 
             }
            );
        }
    }

    template<class MAIN_CMP_t, class ...EXTRA_CMPS_t>
    constexpr auto
    DoForEachComponentType(void (*sys_upd)(MAIN_CMP_t, EXTRA_CMPS_t...)) const -> void
    {
        for (auto& cmp : GetRequieredComponentStorage<MAIN_CMP_t>()) {
            auto& ent { GetEntityByID(cmp.GetEntityID()) };
            std::apply
            (
             sys_upd,
             Elements_t<MAIN_CMP_t, EXTRA_CMPS_t...>
             {
              cmp,
              GetRequieredComponent<EXTRA_CMPS_t>(ent)... 
             }
            );
        }
    }

    template<class SysSignature_t, class Callable_t>
    constexpr auto DoForEachComponentType(Callable_t&& callable)
    {
        using SysExtract = TMP::TypeListExtractor_t<SysSignature_t>;

        SysExtract::template invoke_functor<FunctorHelperExtractor_t>
            (
             *this,
             std::forward<Callable_t>(callable)
            );
    }

private:

    Storage_t<OwnEntity_t> m_Entities {  };
    ComponentStorage_t<CMP_t, CMPS_t...> m_Components {  };
};

} // namespace ECS

#pragma once

#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <queue>

#include "component_storage.hpp"

#include <ecs/util/type_aliases.hpp>
#include <ecs/util/helpers.hpp>
#include <ecs/cmp/entity.hpp>
#include <tmp/type_list.hpp>

//TODO vector of entities for systems
//TODO create unique types for ComponentTypeID, ComponentID, EntityID_t
//TODO Sort when destroy a entity
/*TODO for enhanced loop
 * iterator tat returns a tuple with components 
 */
namespace ECS
{

struct ComponentExtractorHelper_t
{
    template<class MainComponent_t,
             class... ExtraComponents_t,
             class EntManager_t,
             class Callable_t>
    constexpr auto
    operator()(EntManager_t&& ent_man, Callable_t&& callable) -> void
    {
        ent_man.template
        DoForEachComponentTypeIMPL<MainComponent_t,
                                   ExtraComponents_t...>
                                   (std::forward<EntManager_t>(ent_man),
                                    std::forward<Callable_t>(callable));
    }
};
//TODO: Use friend entitybase instead
template<class... EntitiesSignature_t>
class EntityManager_t
{
public:

    using Self_t = EntityManager_t<EntitiesSignature_t...>;

    template<class Systems_t>
    using OwnEntity_t = Entity_t<Self_t, Systems_t>;

    template<class EntitySignature_t>
    using VecEntity_t = Storage_t<OwnEntity_t<EntitySignature_t>>;
    using TableOfEntitites = Elements_t<VecEntity_t<EntitiesSignature_t>...>;

    constexpr explicit EntityManager_t() = default;
    //WIP
    template<class... Systems>
    auto CreateRequieredEntity() -> OwnEntity_t<TMP::TypeList_t<Systems...>>&
    {
        const auto ent_id { mEntities.size() };
        mEntities.push_back(OwnEntity_t{ ent_id });
        return mEntities.back();
    }

    template<class EntitySignature_t>
    constexpr auto
    GetRequieredEntities() const
    -> const VecEntity_t<EntitySignature_t>&
    {
        return std::get<VecEntity_t<EntitySignature_t>>(mEntities);
    }

    template<class EntitySignature_t>
    constexpr auto
    GetRequieredEntities()
    -> VecEntity_t<EntitySignature_t>&
    {
        return SameAsConstMemFunc(this, &EntityManager_t::GetRequieredEntities);
    }

    template<class EntitySignature_t>
    constexpr auto
    GetEntityByID(EntityID_t ent_id) const
    -> const OwnEntity_t<EntitySignature_t>&
    {
        return GetRequieredEntities<EntitySignature_t>()[ent_id];
    }

    template<class EntitySignature_t>
    auto GetEntityByID(EntityID_t ent_id)
    -> OwnEntity_t<EntitySignature_t>&
    {
        return SameAsConstMemFunc(this,
                                  &EntityManager_t::GetEntityByID,
                                  ent_id);
    }

    template<class EntitySignature_t>
    constexpr auto
    RemoveEntityComponents(OwnEntity_t<EntitySignature_t>& e) -> void
    {
        e.ForEachComponentID(
        [](auto&& cmp_id){
            auto eid { mComponents.RemoveRequieredComponent(cmp_id) };
            auto& ent_upd { GetEntityByID<EntitySignature_t>(eid) };
            ent_upd.UpdateRequieredComponentID(cmp_id)
        });
    }

    template<class EntitySignature_t>
    constexpr auto
    UpdateEntityComponentsEntityID(OwnEntity_t<EntitySignature_t>& ent,
                                   EntityID_t new_eid)
    -> void
    {
        ent.ForEachComponentID(
        [&new_id](auto&& cmp_id){
            mComponents.SetComponentEntityID(std::forward(cmp_id), new_eid);
        });
    }

    //TODO: add markedAsDead for prevent the last_entity don't being updated
    template<class EntitySignature_t>
    constexpr auto
    RemoveEntity(const OwnEntity_t<EntitySignature_t>& e) -> void
    {
        auto& ent { GetEntityByID(e.GetEntityID()) };
        auto& last_entity { GetRequieredEntities<EntitySignature_t>().back() };
        RemoveEntityComponents(ent);
        UpdateEntityComponentsEntityID(last_entity, e.GetEntityID());
        ent = std::move(last_entity);
        GetRequieredEntities<EntitySignature_t>().pop_back();
    }

    template<class EntitySignature_t, class InternalComponent_t>
    constexpr auto
    GetEntityByComponent(InternalComponent_t&& in_cmp) const
    -> const OwnEntity_t<EntitySignature_t>&
    {
        return GetEntityByID(in_cmp.EntityID);
    }

    template<class EntitySignature_t, class InternalComponent_t>
    constexpr auto
    GetEntityByComponent(InternalComponent_t&& in_cmp)
    -> OwnEntity_t<EntitySignature_t>&
    {
        return SameAsConstMemFunc(this,
                                  &EntityManager_t::template
                                  GetEntityByComponent<InternalComponent_t>,
                                  in_cmp);
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentStorage() const
    -> const VecComponent_t<RemovePCR<ReqCmp_t>>&
    {
        return mComponents.template
               GetRequieredComponentStorage<RemovePCR<ReqCmp_t>>();
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetRequieredComponentStorage()
    -> VecComponent_t<RemovePCR<ReqCmp_t>>&
    {
        return SameAsConstMemFunc
               (
                 this,
                 &EntityManager_t::
                 GetRequieredComponentStorage<ReqCmp_t>
               );
    }

    template<class ID_Cmp_t>
    constexpr auto
    AttachComponentToEntity(OwnEntity_t& ent,
                            ID_Cmp_t id_cmp)
    -> typename ID_Cmp_t::second_type&
    {
        auto [cmp_id, cmp] { id_cmp };
        const auto cmp_tp_id
        {
            mComponents.template
            GetRequiredComponentTypeID<typename ID_Cmp_t::second_type>()
        };
        ent.AttachComponentID(cmp_tp_id, cmp_id);
        return cmp;
    }

    //FIXME: What happens when the entity already has the ReqCmp_t component
    auto CreateRequieredComponent(OwnEntity_t& ent, Args_t&& ...args)
    -> ReqCmp_t&
    {
        auto id_cmp
        {
            mComponents.template
            CreateRequieredComponent<ReqCmp_t>
            (
             ent.GetEntityID(),
             std::forward<Args_t>(args)...
            )
        };

        return AttachComponentToEntity(ent, id_cmp);
    }

    template<class... ReqCmps_t,
             class TupleCmps_t,
             std::size_t... I>
    constexpr auto
    CreateRequieredComponentsIMPL(OwnEntity_t& ent,
                                  TupleCmps_t&& cmps,
                                  std::index_sequence<I...>)
    -> Elements_t<ReqCmps_t&...>
    {
        return
        {
            AttachComponentToEntity
            (
             ent,
             std::get<I>(std::forward<TupleCmps_t>(cmps))
            )...
        };
    }

    template<class... ReqCmps_t, class... TupleArgs_t>
    constexpr auto
    CreateRequieredComponents(OwnEntity_t& ent, TupleArgs_t&&... args)
    -> Elements_t<ReqCmps_t&...>
    {
        auto cmps
        {
            mComponents.template
            CreateRequieredComponents<ReqCmps_t...>
            (
             ent.GetEntityID(),
             std::forward<TupleArgs_t>(args)...
            )
        };

        constexpr auto indexes
        {
            std::make_index_sequence<std::tuple_size_v
                                    <std::remove_reference_t<decltype(cmps)>>>{}
        };
        return CreateRequieredComponentsIMPL<ReqCmps_t...>(ent, cmps, indexes);
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetOptionalComponent(const OwnEntity_t& ent)
    -> Nullable_t<ReqCmp_t>
    {
        const auto cmp_tp_id
        {
            mComponents.template GetRequiredComponentTypeID<ReqCmp_t>()
        };

        const auto cmp_id
        {
            ent.FindRequiredComponentID(cmp_tp_id)
        };

        Nullable_t<ReqCmp_t> optional_cmp {  };
        if (cmp_id) {
            auto& cmp
            {
                mComponents.template
                GetRequieredComponentByID<ReqCmp_t>(*cmp_id)
            };
            optional_cmp.emplace(cmp);
        }

        return optional_cmp;
    }

    template<typename ReqCmp_t>
    constexpr auto
    GetOptionalComponent(const OwnEntity_t& ent) const
    -> const Nullable_t<const ReqCmp_t>
    {
        return
        const_cast<Self_t*>(this)->GetOptionalComponent<const ReqCmp_t>(ent);
    }

    template<typename OptionalCmpt_t,
             typename ReqCmp_t = typename OptionalCmpt_t::value_type::type>
    constexpr auto
    GetRequieredComponent(const OwnEntity_t& ent) const
    -> const Nullable_t<const ReqCmp_t>
    {
        return GetOptionalComponent<ReqCmp_t>(ent);
    }

    template<typename OptionalCmpt_t,
             typename ReqCmp_t = typename OptionalCmpt_t::value_type::type>
    constexpr auto
    GetRequieredComponent(const OwnEntity_t& ent)
    -> const Nullable_t<ReqCmp_t>
    {
        return GetOptionalComponent<ReqCmp_t>(ent);
    }

    template<typename ReqCmp_t,
             typename std::enable_if_t<IsOneOf<ReqCmp_t,
                                       Components_t...>::value, bool> = true>
    constexpr auto
    GetRequieredComponent(const OwnEntity_t& ent) const
    -> const ReqCmp_t&
    {
        const auto cmp_tp_id
        {
            mComponents.template GetRequiredComponentTypeID<ReqCmp_t>()
        };

        const auto cmp_id
        {
            ent.GetRequiredComponentID(cmp_tp_id)
        };

        return mComponents.template
               GetRequieredComponentByID<ReqCmp_t>(cmp_id);
    }

    template<typename ReqCmp_t,
             typename std::enable_if_t<IsOneOf
                                       <ReqCmp_t,
                                        Components_t...>::value, bool> = true>
    constexpr auto
    GetRequieredComponent(const OwnEntity_t& ent)
    -> ReqCmp_t&
    {
        return SameAsConstMemFunc(this,
                                  &Self_t::template
                                  GetRequieredComponent<ReqCmp_t>,
                                  ent);
    }
    //TODO: add support for Nullable_t
    template<typename ...ReqCmp_t>
    constexpr auto
    GetRequieredComponents([[maybe_unused]]const OwnEntity_t& ent) const
    -> const Elements_t<const ReqCmp_t&...>
    {
        return { GetRequieredComponent<ReqCmp_t>(ent)... };
    }

    template<typename ...ReqCmp_t>
    constexpr auto
    GetRequieredComponents([[maybe_unused]]const OwnEntity_t& ent)
    -> Elements_t<ReqCmp_t&...>
    {
        return { GetRequieredComponent<ReqCmp_t>(ent)... };
    }

    template<class MainCmp_t,
             class... ExtraCmp_t,
             class Callable_t,
             class This_t>
    constexpr auto
    DoForEachComponentTypeIMPL(This_t&& self, Callable_t&& callable)
    -> void
    {
        for (auto& [eid, cmp] :
             self.template GetRequieredComponentStorage<MainCmp_t>()) {
            auto& ent { self.GetEntityByID(eid) };
            std::invoke(std::forward<Callable_t>(callable),
                        cmp,
                        self.template GetRequieredComponent<ExtraCmp_t>(ent)...,
                        ent);
        }
    }

    template<class MainCmp_t,
             class... ExtraCmp_t,
             class Callable_t,
             class This_t>
    constexpr auto
    DoForEachComponentTypeIMPL(This_t&& self, Callable_t&& callable) const
    -> void
    {
        const_cast<Self_t*>
        (this)->template DoForEachComponentTypeIMPL<MainCmp_t,
                                                    ExtraCmp_t...>
                                           (std::forward<This_t>(self),
                                            std::forward<Callable_t>(callable));
    }

    template<class MainCmp_t, class ...ExtraCmp_t>
    constexpr auto
    DoForEachComponentType(void (&sys_upd)(const MainCmp_t,
                                           const ExtraCmp_t...,
                                           const OwnEntity_t&)) const
    -> void
    {
        DoForEachComponentTypeIMPL<RemovePCR<MainCmp_t>,
                                   RemovePCR<ExtraCmp_t>...>(*this,
                                                             sys_upd);
    }

    template<class MainCmp_t, class ...ExtraCmp_t>
    constexpr auto
    DoForEachComponentType(void (&sys_upd)(MainCmp_t,
                                           ExtraCmp_t...,
                                           OwnEntity_t&))
    -> void
    {
        DoForEachComponentTypeIMPL<RemovePCR<MainCmp_t>,
                                   RemovePCR<ExtraCmp_t>...>(*this,
                                                             sys_upd);
    }

    template<class SysSignature_t,
             class Callable_t,
             std::enable_if_t<IsVariadicTemplated<SysSignature_t>::value,
                              bool> = true>
    constexpr auto
    DoForEachComponentType(Callable_t&& callable) const -> void
    {
        using SysExtract = TMP::TypeListExtractor_t<SysSignature_t>;

        SysExtract::template invoke_functor<ComponentExtractorHelper_t>
            (
             *this,
             std::forward<Callable_t>(callable)
            );
    }

    template<class SysSignature_t,
             class Callable_t,
             std::enable_if_t<IsVariadicTemplated<SysSignature_t>::value,
                              bool> = true>
    constexpr auto
    DoForEachComponentType(Callable_t&& callable) -> void
    {
        using SysExtract = TMP::TypeListExtractor_t<SysSignature_t>;

        SysExtract::template invoke_functor<ComponentExtractorHelper_t>
            (
             *this,
             std::forward<Callable_t>(callable)
            );
    }

private:

    TableOfEntitites mEntities {  };
    ComponentStorage_t<Self_t> mComponents {  };
    //std::queue<EntityID> mDeadEntities {  };
};

} // namespace ECS

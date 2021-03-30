#pragma once

#include <utility>

#include <ecs/util/type_aliases.hpp>
#include <ecs/util/helpers.hpp>

namespace ECS
{

template<typename CMP_t, typename ...CMPS_t>
struct ComponentStorage_t final : Uncopyable_t
{

    using ElementsT_t = Elements_t<Storage_t<CMP_t>, Storage_t<CMPS_t>...>;

    constexpr explicit ComponentStorage_t()
    {
        CheckIfComponentsAreUnique();
    }

    template<typename REQ_CMP_t, typename ...Args>
    constexpr
    auto CreateRequieredComponent(EntityID_t eid, Args&& ...args)
                                                        -> RemovePCR<REQ_CMP_t>&
    {
        auto& vec_cmps { GetRequieredComponentStorage<REQ_CMP_t>() };

        const ComponentID_t cmp_id { vec_cmps.size() };
        auto& cmp      { vec_cmps.emplace_back(eid,
                                               cmp_id,
                                               std::forward<Args>(args)...) };

        return cmp;
    }

    template<typename REQ_CMP_t>
    static constexpr auto
    GetRequiredComponentTypeID() -> ComponentTypeID_t
    {
        return IndexOfElement_t<Storage_t<RemovePCR<REQ_CMP_t>>,
                                ElementsT_t>::value;
    }

    template<typename REQ_CMP_t>
    constexpr auto
    GetRequieredComponentStorage() const
                                   -> const Storage_t<RemovePCR<REQ_CMP_t>>&
    {
        CheckIfComponentIsInThisInstance<RemovePCR<REQ_CMP_t>>();

        return std::get<Storage_t<RemovePCR<REQ_CMP_t>>>(m_Components);
    }

    template<typename REQ_CMP_t>
    constexpr auto
    GetRequieredComponentStorage() -> Storage_t<RemovePCR<REQ_CMP_t>>&
    {
        return SameAsConstMemFunc
               (
                this,
                &ComponentStorage_t::template
                GetRequieredComponentStorage<REQ_CMP_t>
               );
    }

    template<typename REQ_CMP_t>
    constexpr auto
    GetRequieredComponent(ComponentID_t cmp_id) const
                                                -> const RemovePCR<REQ_CMP_t>&
    {
        auto& cmps { GetRequieredComponentStorage<REQ_CMP_t>() };

        return cmps[cmp_id];
    }

    template<typename REQ_CMP_t>
    constexpr auto
    GetRequieredComponent(ComponentID_t cmp_id) -> RemovePCR<REQ_CMP_t>&
    {
        return SameAsConstMemFunc(this,
               &ComponentStorage_t::template GetRequieredComponent<REQ_CMP_t>,
               cmp_id);
    }

private:

    template<typename REQ_CMP_t>
    static constexpr auto CheckIfComponentIsInThisInstance() -> void
    {
        static_assert(IsOneOf<REQ_CMP_t, CMP_t, CMPS_t...>::value,
                      "The requiered component type does not"
                      " exist in this ComponentStorage_t");
    }

    static constexpr auto CheckIfComponentsAreUnique()  -> void
    {
        static_assert(AreUnique_t<CMP_t, CMPS_t...>::value,
                      "Components need to be unique");
    }

    ElementsT_t m_Components {  };
};

} // namespace ECS

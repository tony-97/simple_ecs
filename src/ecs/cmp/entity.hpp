#pragma once

#include <unordered_map>
#include <cassert>

#include <ecs/util/type_aliases.hpp>
#include <ecs/util/helpers.hpp>
#include <utility>

namespace ECS
{

struct EntityBase_t
{
protected:

    auto AttachComponentID(ComponentTypeID_t cmp_tp_id,
                           ComponentID_t cmp_id) -> void
    {
        m_Comps[cmp_tp_id] = cmp_id;
    }

    auto FindRequiredComponentID(ComponentTypeID_t cmp_tp_id) const
    -> Optional_t<ComponentID_t>
    {
        Optional_t<ComponentID_t> cmp_id {  };

        auto ite = m_Comps.find(cmp_tp_id);
        if (ite != m_Comps.cend()) {
            cmp_id.emplace(ite->second);
        }

        return cmp_id;
    }

    auto FindRequiredComponentID(ComponentTypeID_t cmp_tp_id)
    -> Optional_t<ComponentID_t>
    {
        return const_cast<const EntityBase_t*>
               (this)->FindRequiredComponentID(cmp_tp_id);
    }

    auto GetRequiredComponentID(ComponentTypeID_t cmp_tp_id) const
    -> ComponentID_t
    {
        assert_msg(FindRequiredComponentID(cmp_tp_id), "The entity doesn't"
                                                       " have the component");
        return const_cast<EntityBase_t*>(this)->m_Comps[cmp_tp_id];
    }

    auto GetRequiredComponentID(ComponentTypeID_t cmp_tp_id)
    -> ComponentID_t
    {
        return const_cast<const EntityBase_t*>
              (this)->GetRequiredComponentID(cmp_tp_id);
    }

    auto UpdateComponentID(ComponentTypeID_t cmp_tp_id, ComponentID_t cmp_id)
    -> void
    {
        m_Comps[cmp_tp_id] = cmp_id;
    }

    decltype(auto) begin()        { return m_Comps.begin(); }
    decltype(auto) begin()  const { return m_Comps.begin(); }
    decltype(auto) cbegin() const { return m_Comps.cbegin(); }
    decltype(auto) end()          { return m_Comps.end(); }
    decltype(auto) end()    const { return m_Comps.end(); }
    decltype(auto) cend()   const { return m_Comps.cend(); }

private:
    std::unordered_map<ComponentTypeID_t, ComponentID_t> m_Comps {  };
};

template<class EntMan_t>
struct Entity_t final : public EntityBase_t,
                               Uncopyable_t
{
    friend EntMan_t;

    explicit constexpr
    Entity_t(Entity_t<EntMan_t>&& ent_temp)
        : EntityBase_t { std::move(ent_temp) },
          m_ID { std::move(ent_temp.m_ID) } {  }

    constexpr auto GetEntityID() const -> EntityID_t
    {
        return m_ID;
    }

    constexpr auto GetEntityID() -> EntityID_t
    {
        return m_ID;
    }

private:

    explicit constexpr Entity_t(EntityID_t ent_id) : m_ID { ent_id } {  };

    constexpr
    auto operator=(const Entity_t<EntMan_t>& other)
    -> Entity_t&
    {
        EntityBase_t::operator=(std::move(other));
        return *this;
    }

    EntityID_t m_ID {  };
};

} // namespace ECS

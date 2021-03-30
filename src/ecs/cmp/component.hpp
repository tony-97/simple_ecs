#pragma once

#include "ecs/util/helpers.hpp"
#include <ecs/util/identificable.hpp>
#include <ecs/util/type_aliases.hpp>

namespace ECS
{

struct ComponentBase_t
{
    explicit constexpr ComponentBase_t(EntityID_t ent_id, ComponentID_t self_id)
        : m_EntID { ent_id }, m_ID { self_id }
    {  }

    virtual ~ComponentBase_t() = default;

    constexpr auto GetComponentID() const -> ComponentTypeID_t
    {
        return m_ID;
    }

    constexpr auto GetComponentID() -> ComponentTypeID_t
    {
        return m_ID;
    }

    constexpr auto GetEntityID() const -> EntityID_t
    {
        return m_EntID;
    }

    constexpr auto GetEntityID() -> EntityID_t
    {
        return m_EntID;
    }

private:
    const EntityID_t m_EntID {  };
    const ComponentID_t m_ID {  };
};

} // namespace ECS

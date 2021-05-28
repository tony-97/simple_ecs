#pragma once

#include "sys.hpp"

#include <game/cmp/physics.hpp>

struct PhysicsSystem_t : SystemBase_t<PhysicsComponent_t>
{
    template<class EntMan_t>
    void Update(EntMan_t&& ent_man)
    {
        ent_man.template DoForEachComponentType<SystemSignature_t>(
                [](PhysicsComponent_t& phy,
                   auto&) {
                    phy.pos += phy.vel;
                });
    }
};


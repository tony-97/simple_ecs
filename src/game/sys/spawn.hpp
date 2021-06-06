#pragma once

#include "sys.hpp"

#include <game/cmp/spawn.hpp>
#include <game/cmp/render.hpp>
#include <game/cmp/collider.hpp>
#include <game/cmp/physics.hpp>

#include <raylib.h>

struct SpawnSystem_t : SystemBase_t<SpawnComponent_t, PhysicsComponent_t>
{
    template<class EntMan>
    void Update(EntMan&& ent_man)
    {
        using namespace std::chrono;

        auto now { steady_clock::now() };
        ent_man.template DoForEachComponentType<SystemSignature_t>(
                [&now](SpawnComponent_t& spw,
                                       const PhysicsComponent_t& phy,
                                       auto&){
                    auto passed { now - spw.last_spawn_time };
                    if (spw.to_be_spawned > 0 && passed > spw.spawn_interval) {
                        spw.spawn_callable(phy.pos.x, phy.pos.y);
                        spw.last_spawn_time = now;
                        --spw.to_be_spawned;
                    }
                });
    }
};

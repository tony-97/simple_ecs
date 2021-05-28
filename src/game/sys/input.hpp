#pragma once

#include "raylib.h"
#include "sys.hpp"

#include <game/cmp/input.hpp>
#include <game/cmp/physics.hpp>

struct InputSystem_t : SystemBase_t<InputComponent_t, PhysicsComponent_t>
{
    template<class EntMan>
    void Update(EntMan&& ent_man)
    {
        ent_man.template DoForEachComponentType<SystemSignature_t>(
                [](const InputComponent_t& inp,
                   PhysicsComponent_t& phy,
                   auto&){
                    phy.vel *= 0;
                    if (IsKeyDown(inp.k_down)) phy.vel.y = +5;
                    if (IsKeyDown(inp.k_up))   phy.vel.y = -5;
                    if (IsKeyDown(inp.k_left)) phy.vel.x = -5;
                    if (IsKeyDown(inp.k_right))phy.vel.x = +5;
                });
    }
};


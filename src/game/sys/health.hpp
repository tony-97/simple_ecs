#pragma once

#include "sys.hpp"

#include <game/cmp/health.hpp>
#include <game/cmp/collider.hpp>
#include <iostream>
struct HealthSystem_t : SystemBase_t<HealthComponent_t,
                                     ColliderComponent_t>
{
    bool LeafNodeCollided(const BoundingBoxNode_t& bxn)
    {
        if (bxn.root.empty()) {
            return bxn.collided;
        } else {
            for (auto& bn : bxn.root) {
                if (LeafNodeCollided(bn)) {
                    return true;
                }
            }
        }
        return false;
    }

    template<class EntMan>
    constexpr void
    Update(EntMan&& ent_man)
    {
        ent_man.template DoForEachComponentType<SystemSignature_t>(
                [this, &ent_man](HealthComponent_t& hel,
                   ColliderComponent_t& col,
                   auto& e){
                    if (hel.health && LeafNodeCollided(col.BoxRoot)) {
                        if (--hel.health == 0) {
                            ent_man.RemoveEntity(e);
                            std::cout << "Entity: "
                                      << e.GetEntityID()
                                      << " is dead!"
                                      << std::endl;
                        } else {
                            std::cout << "Entity "
                                      << e.GetEntityID()
                                      << "[HEALTH]: "
                                      << hel.health
                                      << std::endl;
                            }
                    }
                });
    }
};

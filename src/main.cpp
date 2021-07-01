#include <raylib.h>

#include <ecs/man/entity_manager.hpp>

#include <game/util/gamefactory.hpp>
#include <type_traits>

int main()
{
    ECS::EntityManager_t<PhysicsSystem_t,
                         RenderSystem_t,
                         InputSystem_t,
                         ColliderSystem_t,
                         HealthSystem_t,
                         SpawnSystem_t> ent_man {  };

    const
    RenderSystem_t   ren_sys { 640, 480, "Game"  };
    PhysicsSystem_t  phy_sys {  };
    InputSystem_t    inp_sys {  };
    ColliderSystem_t col_sys { 640, 480 };
    HealthSystem_t   hel_sys {  };
    SpawnSystem_t    spw_sys {  };

    ren_sys.ToggleDebugRender();

    GameFactory_t go_fact { ent_man };

    go_fact.CreateSpawner(50, 50,
            [&go_fact](int x, int y) {
                    go_fact.CreateBlade(x, y);
            });
    go_fact.CreateRandomBlade(640, 480);
    go_fact.CreateRandomBlade(640, 480);
    go_fact.CreatePlatform(500, 400);
    go_fact.CreatePlayer(640, 480);

    while (ren_sys.Update(ent_man)) {
        spw_sys.Update(ent_man);
        phy_sys.Update(ent_man);
        col_sys.Update(ent_man);
        hel_sys.Update(ent_man);
        inp_sys.Update(ent_man);
    }

    return 0;
}

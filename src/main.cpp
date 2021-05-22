#include <ecs/man/entity_manager.hpp>
#include <tmp/type_list.hpp>

#include <game/sys/sys.hpp>

#include <iostream>

struct PositionComponent_t
{
    int x {  };
    int y {  };
};

struct PhysicsSystem_t : SystemBase_t<PositionComponent_t>
{
    template<class EntMan_t>
    void Update(EntMan_t&& ent_man)
    {
        ent_man.template DoForEachComponentType<SystemSignature_t>(
                [&](const PositionComponent_t& pos){
                    std::cout << "X is: " << pos.x << " Y is: " << pos.y << std::endl;
                });
    }
};


int main()
{
    ECS::EntityManager_t<PositionComponent_t> EntMan {  };
    auto& ent { EntMan.CreateEntity() };
    EntMan.CreateRequieredComponent<PositionComponent_t>(ent, 3, 4);

    PhysicsSystem_t phy_sys {  };
    phy_sys.Update(EntMan);
    std::cout << "Done...!" << std::endl;
    return 0;
}

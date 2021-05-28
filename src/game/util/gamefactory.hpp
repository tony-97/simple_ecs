#pragma once

#include <game/sys/collider.hpp>
#include <game/sys/input.hpp>
#include <game/sys/physics.hpp>
#include <game/sys/render.hpp>
#include <game/sys/spawn.hpp>

template<class EntityManager_t>
struct GameFactory_t
{
    GameFactory_t(EntityManager_t& ent_man) : m_EntMan { ent_man } {  }

    decltype(auto) CreateEntity(Texture2D& sp,
                                int px,
                                int py,
                                float sz = 5.0f)
    {
        auto& ent { m_EntMan.CreateEntity() };
        auto& ren
        {
            m_EntMan.template
            CreateRequieredComponent<RenderComponent_t>(ent, sp, sz)
        };

        m_EntMan.template
        CreateRequieredComponent<PhysicsComponent_t>(ent,
                                                     VecInt{ px, py },
                                                     VecInt{ 5, 7 } );

        auto& col
        {
            m_EntMan.template
            CreateRequieredComponent<ColliderComponent_t>(ent)
        };

        col.BoxRoot.box.xRight = ren.wh.x;
        col.BoxRoot.box.xLeft  = 0;
        col.BoxRoot.box.yUp    = 0;
        col.BoxRoot.box.yDown  = ren.wh.y;
        return ent;
    }

    decltype(auto) CreateRandomEntity(Texture2D& sp,
                                      int wh,
                                      int hg,
                                      float sz = 5.0f)
    {
        const int x = GetRandomValue(0, wh - sp.width / sz);
        const int y = GetRandomValue(0, hg - sp.height / sz);
        auto& e = CreateEntity(sp, x, y, sz);
        return e;
    }

    template<typename Callable_t>
    decltype(auto) CreateSpawner(int x, int y, Callable_t&& cb)
    {
        auto& ent { m_EntMan.CreateEntity() };
        m_EntMan.template
        CreateRequieredComponent<PhysicsComponent_t>(ent,
                                                     VecInt{ x, y },
                                                     VecInt{ 0, 1 });
        m_EntMan.template
        CreateRequieredComponent<SpawnComponent_t>(ent,
                                                   std::forward<Callable_t>
                                                   (cb));
        ColliderComponent_t& col
        {
            m_EntMan.template
            CreateRequieredComponent<ColliderComponent_t>(ent)
        };
        col.mask = 0X0;

        return ent;
    }

    decltype(auto) CreateBlade(int px, int py)
    {
        static Texture2D blade  { LoadTexture("assets/blade.png") };
        return CreateEntity(blade, px, py, 7.0f);
    }

    decltype(auto) CreateRandomBlade(int wh, int hg)
    {
        static Texture2D blade  { LoadTexture("assets/blade.png") };
        return CreateRandomEntity(blade, wh, hg, 7.0f);
    }

    decltype(auto) CreatePlayer(int wh, int hg, float sz = 5.0f)
    {
        static Texture2D player { LoadTexture("assets/sprite.png") };
        auto& ent { CreateRandomEntity(player, wh, hg, sz) };

        m_EntMan.template
        CreateRequieredComponent<InputComponent_t>(ent);

        ColliderComponent_t& col
        {
            m_EntMan.template
            GetRequieredComponent<ColliderComponent_t>(ent)
        };

        col.mask = 0X0;
        col.BoxRoot.root = { { 
                               {
                                   { {}, BoxFromSize(140, 355, 20 , 90, sz) },
                                   { {}, BoxFromSize(102, 354, 103,238, sz) }
                               },
                               { BoxFromSize(86 , 366, 13 , 250, sz) }
                             },
                             { {}, { BoxFromSize(6  , 89 , 260, 428, sz) } },
                             { {}, { BoxFromSize(96 , 302, 266, 380, sz) } },
                             { {}, { BoxFromSize(66 , 395, 394, 546, sz) } },
                             { {}, { BoxFromSize(317, 476, 267, 408, sz) } },
                             { 
                                 {
                                   { {}, BoxFromSize(423, 545, 225, 343, sz) },
                                   { {}, BoxFromSize(557, 600, 245, 326, sz) },
                                   { {}, BoxFromSize(611, 673, 247, 303, sz) },
                                 },
                                 { BoxFromSize(406, 686, 209, 355, sz) }
                             } };

        return ent;
    }

    constexpr auto
    BoxFromSize(int xLeft, int xRight, int yUp, int yDown, float sz)
    -> BoundingBox_t
    {
        return { static_cast<unsigned>(xLeft / sz),
                 static_cast<unsigned>(xRight / sz),
                 static_cast<unsigned>(yUp / sz),
                 static_cast<unsigned>(yDown / sz) };
    }

private:
    EntityManager_t& m_EntMan {  };
};

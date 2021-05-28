#pragma once

#include "sys.hpp"

#include <game/cmp/collider.hpp>
#include <game/cmp/physics.hpp>
#include <iostream>
struct ColliderSystem_t : SystemBase_t<ColliderComponent_t, PhysicsComponent_t>
{
    ColliderSystem_t(unsigned wh, unsigned hg)
        : width { wh }, height { hg } {  }

    BoundingBox_t Transform2WorldCoordinates(const BoundingBox_t& box,
                                              const PhysicsComponent_t& phy) const
    {
        auto xl { box.xLeft  + phy.pos.x };
        auto xr { box.xRight + phy.pos.x };
        auto yu { box.yUp    + phy.pos.y };
        auto yd { box.yDown  + phy.pos.y };
        return { xl, xr, yu, yd };
    }

    void CheckScreenCollision(const ColliderComponent_t& col,
                              PhysicsComponent_t& phy) const
    {
        auto box { Transform2WorldCoordinates(col.BoxRoot.box, phy) };
        if (box.xRight > width || box.xLeft > width) {
            phy.pos.x -= phy.vel.x;
            phy.vel.x = -phy.vel.x;
        }
        if (box.yDown > height || box.yUp > height) {
            phy.pos.y -= phy.vel.y;
            phy.vel.y = -phy.vel.y;
        }
    }

    bool CheckBoundingBoxCollision(const BoundingBox_t& b1,
                                   const BoundingBox_t& b2) const
    {
        auto range_col {
            [](auto l1, auto r1, auto l2, auto r2){
                if (l2 > r1) return false;
                if (l1 > r2) return false;
                return true;
            }
        };

        return    range_col(b1.xLeft, b1.xRight, b2.xLeft, b2.xRight)
               && range_col(b1.yUp, b1.yDown, b2.yUp, b2.yDown);
    }

    void CheckBoundingBoxNodeCollision(BoundingBoxNode_t& bn1,
                                       BoundingBoxNode_t& bn2,
                                       const PhysicsComponent_t& ph1,
                                       const PhysicsComponent_t& ph2) const
    {
        auto bx1 { Transform2WorldCoordinates(bn1.box, ph1) };
        auto bx2 { Transform2WorldCoordinates(bn2.box, ph2) };

        bool collided { CheckBoundingBoxCollision(bx1, bx2) };

        if (collided) {
            if (!bn1.root.empty()) {
                for (auto& bx : bn1.root) {
                    CheckBoundingBoxNodeCollision(bx, bn2, ph1, ph2);
                }
            } else if (!bn2.root.empty()) {
                for (auto& bx : bn2.root) {
                    CheckBoundingBoxNodeCollision(bn1, bx, ph1, ph2);
                }
            } else {
                bn1.collided = bn2.collided = true;
            }   
        }
    }

    void ResetCollidedFlag(BoundingBoxNode_t& bn) const
    {
        bn.collided = false;
        for (auto& bx : bn.root) {
            ResetCollidedFlag(bx);
        }
    }

    template<class EntMan>
    void Update(EntMan&& ent_man) const
    {
        auto& vec_col
        {
            ent_man.template GetRequieredComponentStorage<ColliderComponent_t>()
        };

        for (auto& col : vec_col) { ResetCollidedFlag(col.Self.BoxRoot); }

        for (auto ite1 { vec_col.begin() }; ite1 != vec_col.end(); ++ite1) {
            auto& ent1 { ent_man.GetEntityByComponent(*ite1) };
            auto& phy1
            {
                ent_man.template GetRequieredComponent<PhysicsComponent_t>(ent1)
            };
            auto& col1 { ite1->Self };
            CheckScreenCollision(col1, phy1);
            for (auto ite2 { std::next(ite1) }; ite2 != vec_col.end(); ++ite2) {
                auto& ent2 { ent_man.GetEntityByComponent(*ite2) };
                auto& phy2
                {
                    ent_man.template
                    GetRequieredComponent<PhysicsComponent_t>(ent2)
                };
                auto& col2 { ite2->Self };
                if ((col1.mask & col2.mask) == 0) {
                    CheckBoundingBoxNodeCollision(col1.BoxRoot, col2.BoxRoot,
                                                  phy1, phy2);
                }
            }
        }
    }

    unsigned width  {  };
    unsigned height {  };
};

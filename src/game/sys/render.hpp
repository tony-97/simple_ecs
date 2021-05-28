#pragma once

#include "sys.hpp"

#include <game/cmp/render.hpp>
#include <game/cmp/physics.hpp>
#include <game/cmp/collider.hpp>

#include <raylib.h>

class RenderSystem_t : SystemBase_t<RenderComponent_t,
                                    PhysicsComponent_t>
{
public:
    RenderSystem_t(unsigned wh, unsigned hg, const char* name)
    {
        InitWindow(wh, hg, name);
        SetTargetFPS(60);
    }

    ~RenderSystem_t()
    {
        CloseWindow();
    }

    void DrawOneEntity(const RenderComponent_t& ren,
                       const PhysicsComponent_t& phy) const
    {
        const Vector2 pos { static_cast<float>(phy.pos.x),
                            static_cast<float>(phy.pos.y) };

        const float wh { static_cast<float>(ren.sprite.width) };
        const float hg { static_cast<float>(ren.sprite.height) };
        const Rectangle rec_src  { 0, 0, wh, hg };
        const Rectangle rec_dest { pos.x, pos.y, ren.wh.x, ren.wh.y };
        const Vector2 ori_vec { 0, 0 };
        DrawTexturePro(ren.sprite, rec_src, rec_dest,
                       ori_vec, 0.0f, WHITE);
    }

    Rectangle
    RectangleFromBoundingBox(const BoundingBox_t& box, int x, int y) const
    {
        const auto rx { box.xLeft  + x };
        const auto ry { box.yUp + y };
        const auto wh { box.xRight - box.xLeft };
        const auto hg { box.yDown  - box.yUp };
        const Rectangle rec { static_cast<float>(rx),
                              static_cast<float>(ry),
                              static_cast<float>(wh),
                              static_cast<float>(hg) };
        return rec;
    }

    void DrawBox(const BoundingBox_t& box, int x, int y, const Color& c) const
    {
        auto rec { RectangleFromBoundingBox(box, x, y) };
        DrawRectangleLinesEx(rec, 2.0f, c);
    }

    void
    DrawBoxNode(const BoundingBoxNode_t& b_node,
                int x, int y, const Color& c) const
    {
        if (b_node.collided) {
            auto rec{ RectangleFromBoundingBox(b_node.box, x, y) };
            DrawRectangle(rec.x, rec.y, rec.width, rec.height, c);    
        }
        DrawBox(b_node.box, x, y, c);
        for (auto& box : b_node.root) {
            DrawBoxNode(box, x, y, GetColor(ColorToInt(c) >> 1));
        }
    }

    template<class EntMan>
    bool Update(EntMan&& ent_man) const
    {
        BeginDrawing();
        ClearBackground(WHITE);
        DrawFPS(10, 10);
        ent_man.template DoForEachComponentType<SystemSignature_t>(
                [&](const RenderComponent_t& ren,
                    const PhysicsComponent_t& phy,
                    auto& ent){

                    DrawOneEntity(ren, phy);
                    if (m_DebugFlag) {
                        auto& col
                        {
                            ent_man.template
                            GetRequieredComponent<ColliderComponent_t>(ent)
                        };
                        DrawBoxNode(col.BoxRoot,
                                    phy.pos.x, phy.pos.y, m_DebugColor);

                        
                    }
                });
        EndDrawing();

        return !WindowShouldClose();
    }

    constexpr auto
    ToggleDebugRender() const
    ->void
    {
        m_DebugFlag = !m_DebugFlag;
    }

private:
    mutable bool m_DebugFlag { false };
    mutable Color m_DebugColor { RED };
};

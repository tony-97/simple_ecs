#include <tuple>
#include <tmp/type_list.hpp>

#include <game/cmp/component.hpp>
#include <raylib.h>

template<class MainComponent_t, class... ExtraComponents_t>
struct SystemBase_t
{
    virtual ~SystemBase_t() = default;
    using SystemSignature_t = TMP::TypeList_t<MainComponent_t, ExtraComponents_t...>;
};

constexpr int ScreenWidth  = 0;
constexpr int ScreenHeight = 0;

template<class GameEntityManager>
static void CreateBall(GameEntityManager&& ent_man, const Texture2D& sprite)
{
    auto& ent = ent_man.CreateEntity();
    auto& phy = ent_man.template CreateRequieredComponent<PhysicsComponent_t>(ent);
    float posx { static_cast<float>(GetRandomValue(0, ScreenWidth) ) };
    float posy { static_cast<float>(GetRandomValue(0, ScreenHeight)) };
    phy.pos = { posx, posy };
    float velx { static_cast<float>(GetRandomValue(-100, 100)) };
    float vely { static_cast<float>(GetRandomValue(-100, 100)) };
    phy.vel = { velx, vely };
    auto& ren = ent_man.template CreateRequieredComponent<RenderComponent_t>(ent, sprite);
    auto& col = ent_man.template CreateRequieredComponent<ColliderComponent_t>(ent);
    col.m_Collide.m_Top    = { 0.0f, 0.0f }; 
    col.m_Collide.m_Bottom = { ren.scale, ren.scale };
    [[maybe_unused]]auto& sc_col = ent_man.template CreateRequieredComponent<ScreenColliderComponent_t>(ent);
}

inline void DrawEntities(const RenderComponent_t& ren,
                         const PhysicsComponent_t& phy,
                         const ECS::Nullable_t<const ScreenColliderComponent_t> sc_col)
{
    const Rectangle src_rec { 0.0f, 0.0f,
                              static_cast<float>(ren.sprite.width),
                              static_cast<float>(ren.sprite.height) };

    const Rectangle dest_rec { phy.pos.x, phy.pos.y,
                               ren.scale, ren.scale };

    const Vector2   origin_vec2 { 0.0f,
                                  0.0f };

    DrawTexturePro(ren.sprite, src_rec, dest_rec, origin_vec2, 0.0f, ren.tint);

    //DrawRectangleLinesEx(dest_rec, 2.0f, ORANGE);

    if (sc_col) {
        if (sc_col->get().m_ScreenCollision) {
            const auto wrapped_pos { sc_col->get().m_ScreenCollision.value() };
            const Rectangle wrapped_rec { wrapped_pos.x, wrapped_pos.y, dest_rec.width, dest_rec.height };
            //DrawRectangleLinesEx(wrapped_rec, 2.0f, BLUE);
            DrawTexturePro(ren.sprite, src_rec, wrapped_rec, origin_vec2, 0.0f, RED);
        }
    }
}

template<class GameEntityManager>
static void UpdateRenderSystem(const GameEntityManager& ent_man)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    ent_man.DoForEachComponentType(DrawEntities);
    DrawFPS(10, 40);
    EndDrawing();
}

template<typename T>
static void Limits(T& val, Vector_t<T> range)
{
    if (val < range.x) {
        val += range.y;
    }
    if (val > range.y) {
        val -= range.y;
    }
}

inline Vector_t<float> WrapCoordinates(Vector_t<float> pos)
{
    Limits(pos.x, { 0, ScreenWidth  });
    Limits(pos.y, { 0, ScreenHeight });

    return pos;
}

inline void UpdatePhysicsSystem(PhysicsComponent_t& phy)
{
    phy.pos += phy.vel * GetFrameTime();
}

inline void UpdateScreenColliderSystem(ScreenColliderComponent_t& sc_col,
                                       PhysicsComponent_t& phy,
                                       const ColliderComponent_t& col)
{
    auto xl { phy.pos.x + col.m_Collide.m_Top.x };
    auto xr { xl + col.m_Collide.m_Bottom.x     };
    auto yu { phy.pos.y + col.m_Collide.m_Top.y };
    auto yd { yu + col.m_Collide.m_Bottom.y     };

    if (xr < 0) { phy.pos.x += ScreenWidth; }
    else if (xl > ScreenWidth) { phy.pos.x -= ScreenWidth; }

    if (yd < 0) { phy.pos.y += ScreenHeight; }
    else if (yu > ScreenHeight) { phy.pos.y -= ScreenHeight; }

    std::optional<float> wrapped_x {  };
    if (xl < 0) {
        wrapped_x.emplace(xl + ScreenWidth);
    } else if (xr > ScreenWidth) {
        wrapped_x.emplace(xl - ScreenWidth);
    }

    std::optional<float> wrapped_y {  };
    if (yu < 0) {
        wrapped_y.emplace(yu + ScreenHeight);
    } else if (yd > ScreenHeight) {
        wrapped_y.emplace(yu - ScreenHeight);
    }

    Vector2 wrapped_pos { phy.pos.x, phy.pos.y };
    if (wrapped_x) {
        wrapped_pos.x = *wrapped_x;
    }

    if (wrapped_y) {
        wrapped_pos.y = *wrapped_y;
    }

    if (wrapped_pos.x != phy.pos.x || wrapped_pos.y != phy.pos.y) {
        sc_col.m_ScreenCollision.emplace(wrapped_pos);
    } else {
        sc_col.m_ScreenCollision.reset();
    }

}

inline void UpdateInputSystem(const InputComponent_t& inp,
                              PhysicsComponent_t& phy)
{
    phy.vel = { 0.0f, 0.0f };
    constexpr float vel { 100.0f };
    if (IsKeyDown(inp.key_down))  { phy.vel.y = +vel; }
    if (IsKeyDown(inp.key_up  ))  { phy.vel.y = -vel; }
    if (IsKeyDown(inp.key_right)) { phy.vel.x = +vel; }
    if (IsKeyDown(inp.key_left))  { phy.vel.x = -vel; }
}

#include "math_vector.hpp"

#include <ecs/cmp/component.hpp>
#include <ecs/man/entity_manager.hpp>

#include <optional>
#include <raylib.h>

constexpr unsigned ScreenWidth  = 640;
constexpr unsigned ScreenHeight = 480;

struct PhysicsComponent_t final : ECS::ComponentBase_t
{
    explicit PhysicsComponent_t(ECS::EntityID_t ent_id,
                                ECS::ComponentID_t self_id) 
        : ComponentBase_t(ent_id, self_id) {  }

    Vector_t<float> pos {  };
    Vector_t<float> vel {  };
};

struct RenderComponent_t final : ECS::ComponentBase_t
{
    explicit RenderComponent_t(ECS::EntityID_t ent_id,
                               ECS::ComponentID_t self_id,
                               const Texture2D& sp,
                               const Color& co = RAYWHITE)
        : ComponentBase_t(ent_id, self_id), sprite { sp }, tint { co } {  }

    const Texture2D& sprite;
    const Color tint { RAYWHITE };
    float scale { 40 };
};

struct BoundingBox_t
{
    Vector_t<float> m_Top {  };
    Vector_t<float> m_Bottom {  };
};

struct ColliderComponent_t final : ECS::ComponentBase_t
{
    explicit ColliderComponent_t(ECS::EntityID_t ent_id,
                                 ECS::ComponentID_t self_id) 
        : ComponentBase_t(ent_id, self_id) {  }

    BoundingBox_t m_Collide {  };
};

struct ScreenColliderComponent_t final : ECS::ComponentBase_t
{
    explicit ScreenColliderComponent_t(ECS::EntityID_t ent_id,
                                       ECS::ComponentID_t self_id) 
        : ComponentBase_t(ent_id, self_id) {  }

    std::optional<Vector2> m_ScreenCollision {  };
};

struct InputComponent_t final : ECS::ComponentBase_t
{
    explicit InputComponent_t(ECS::EntityID_t ent_id, ECS::ComponentID_t self_id)
        : ComponentBase_t(ent_id, self_id) {  }

    KeyboardKey key_left  { KEY_A };
    KeyboardKey key_right { KEY_D };
    KeyboardKey key_up    { KEY_W };
    KeyboardKey key_down  { KEY_S };
};

using GameEntityManager = ECS::EntityManager_t<PhysicsComponent_t,
                                               RenderComponent_t,
                                               ScreenColliderComponent_t,
                                               InputComponent_t,
                                               ColliderComponent_t>;

static void CreateBall(GameEntityManager& ent_man, const Texture2D& sprite)
{
    auto& ent = ent_man.CreateEntity();
    auto& phy = ent_man.CreateRequieredComponent<PhysicsComponent_t>(ent);
    float posx { static_cast<float>(GetRandomValue(0, ScreenWidth) ) };
    float posy { static_cast<float>(GetRandomValue(0, ScreenHeight)) };
    phy.pos = { posx, posy };
    float velx { static_cast<float>(GetRandomValue(-100, 100)) };
    float vely { static_cast<float>(GetRandomValue(-100, 100)) };
    phy.vel = { velx, vely };
    auto& ren = ent_man.CreateRequieredComponent<RenderComponent_t>(ent, sprite);
    auto& col = ent_man.CreateRequieredComponent<ColliderComponent_t>(ent);
    col.m_Collide.m_Top    = { 0.0f, 0.0f }; 
    col.m_Collide.m_Bottom = { ren.scale, ren.scale };
    [[maybe_unused]]auto& sc_col = ent_man.CreateRequieredComponent<ScreenColliderComponent_t>(ent);
}

static void CreatePlayer(GameEntityManager& ent_man, const Texture2D& sprite)
{
    auto& ent = ent_man.CreateEntity();
    ent_man.CreateRequieredComponent<InputComponent_t>(ent);
    auto& phy = ent_man.CreateRequieredComponent<PhysicsComponent_t>(ent);
    float posx { static_cast<float>(GetRandomValue(0, ScreenWidth) ) };
    float posy { static_cast<float>(GetRandomValue(0, ScreenHeight)) };
    phy.pos = { posx, posy };
    float velx { static_cast<float>(GetRandomValue(-100, 100)) };
    float vely { static_cast<float>(GetRandomValue(-100, 100)) };
    phy.vel = { velx, vely };
    auto& ren = ent_man.CreateRequieredComponent<RenderComponent_t>(ent, sprite, BLUE);
    auto& col = ent_man.CreateRequieredComponent<ColliderComponent_t>(ent);
    col.m_Collide.m_Top    = { 0.0f, 0.0f }; 
    col.m_Collide.m_Bottom = { ren.scale, ren.scale };
    [[maybe_unused]]auto& sc_col = ent_man.CreateRequieredComponent<ScreenColliderComponent_t>(ent);
}

static void DrawEntities(const RenderComponent_t& ren,
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

static int counter = 0;

static void UpdateRenderSystem(const GameEntityManager& ent_man)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    ent_man.DoForEachComponentType(DrawEntities);
    DrawFPS(10, 40);
    DrawText(TextFormat("Entities: %d", counter), 10, 80, 30, BLACK);
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

Vector_t<float> WrapCoordinates(Vector_t<float> pos)
{
    Limits(pos.x, { 0, ScreenWidth  });
    Limits(pos.y, { 0, ScreenHeight });

    return pos;
}

static void UpdatePhysicsSystem(PhysicsComponent_t& phy)
{
    phy.pos += phy.vel * GetFrameTime();
}

static void UpdateScreenColliderSystem(ScreenColliderComponent_t& sc_col,
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

static void UpdateInputSystem(const InputComponent_t& inp,
                              PhysicsComponent_t& phy)
{
    phy.vel = { 0.0f, 0.0f };
    constexpr float vel { 100.0f };
    if (IsKeyDown(inp.key_down))  { phy.vel.y = +vel; }
    if (IsKeyDown(inp.key_up  ))  { phy.vel.y = -vel; }
    if (IsKeyDown(inp.key_right)) { phy.vel.x = +vel; }
    if (IsKeyDown(inp.key_left))  { phy.vel.x = -vel; }
}


int main()
{
    InitWindow(ScreenWidth, ScreenHeight, "Asteroid 3.0");
    SetTargetFPS(4096);

    Texture2D sprite = LoadTexture("../assets/wabbit_alpha.png");
    GameEntityManager ent_man {  };
    for (std::size_t i = 0; i < 10000; ++i) {
        CreateBall(ent_man, sprite);
        ++counter;
    }
    CreatePlayer(ent_man, sprite);
    while (!WindowShouldClose()) {
        UpdateRenderSystem(ent_man);
        ent_man.DoForEachComponentType(UpdateInputSystem);
        ent_man.DoForEachComponentType(UpdatePhysicsSystem);
        ent_man.DoForEachComponentType(UpdateScreenColliderSystem);
        ent_man.DoForEachComponentType<MPL::TypeList_t<RenderComponent_t, PhysicsComponent_t>>([](RenderComponent_t&, PhysicsComponent_t&){});
    }

    CloseWindow();

    return 0;
}

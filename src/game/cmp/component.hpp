#pragma once

#include <raylib.h>

#include <math_vector.hpp>

#include <ecs/cmp/component.hpp>

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

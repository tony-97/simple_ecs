#pragma once

#include <functional>
#include <raylib.h>

struct RenderComponent_t
{
    RenderComponent_t(Texture2D& sp, float sz)
        : sprite { sp }, wh { sp.width / sz, sp.height / sz } {  }

    std::reference_wrapper<Texture2D> sprite;
    Vector2 wh {  };
};

#pragma once

#include <raylib.h>

struct RenderComponent_t
{
    RenderComponent_t(const Texture2D& sp, float sz)
        : sprite { sp }, wh { sp.width / sz, sp.height / sz } {  }

    const Texture2D& sprite;
    Vector2 wh {  };
};

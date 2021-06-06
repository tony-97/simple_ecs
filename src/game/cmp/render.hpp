#pragma once

#include <raylib.h>

struct RenderComponent_t
{
    RenderComponent_t(Texture2D& sp, float sz)
        : sprite { sp }, wh { sp.width / sz, sp.height / sz } {  }

    RenderComponent_t(const RenderComponent_t& other)
        : sprite { other.sprite }, wh { other.wh } {  }

    RenderComponent_t& operator=(const RenderComponent_t& rhs)
    {
        sprite = rhs.sprite;
        wh = rhs.wh;
        return *this;
    }

    Texture2D& sprite;
    Vector2 wh {  };
};

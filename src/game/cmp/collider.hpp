#pragma once

#include <vector>
#include <cstdint>

struct BoundingBox_t
{
    unsigned xLeft  {  };
    unsigned xRight {  };
    unsigned yUp    {  };
    unsigned yDown  {  };
};

struct BoundingBoxNode_t
{
    std::vector<BoundingBoxNode_t> root {  };
    BoundingBox_t box {  };
    bool collided { false };
};

struct ColliderComponent_t
{
    enum
    {
        LNONE     = 0X0,
        LBLADES   = 0x1,
        LPLATFORM = 0x2,
        LALL      = 0xFF
    };

    BoundingBoxNode_t BoxRoot {  };
    std::uint8_t mask { LALL };
};

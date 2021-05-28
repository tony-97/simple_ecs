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
    BoundingBoxNode_t BoxRoot {  };
    std::uint8_t mask { 0XFF };
};



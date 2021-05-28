#pragma once

#include "math_vector.hpp"

using VecInt = Vector_t<int>;

struct PhysicsComponent_t
{
    VecInt pos {  };
    VecInt vel {  };
};

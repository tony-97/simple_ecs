#pragma once

#include <raylib.h>

struct InputComponent_t
{
    KeyboardKey k_left  { KEY_A };
    KeyboardKey k_right { KEY_D };
    KeyboardKey k_up    { KEY_W };
    KeyboardKey k_down  { KEY_S };
};

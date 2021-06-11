#pragma once

#include <chrono>
#include <functional>

struct SpawnComponent_t
{
    using Clock = std::chrono::steady_clock;

    std::function<void(int x, int y)> spawn_callable {  };
    Clock::time_point last_spawn_time { Clock::now() };
    std::chrono::duration<float> spawn_interval { std::chrono::seconds { 5 } };
    unsigned to_be_spawned { 2 }; 
};

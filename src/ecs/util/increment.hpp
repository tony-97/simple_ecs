#pragma once

namespace ECS
{

template<typename T>
struct PreIncrement
{
    constexpr auto operator()(T& t) const -> const T&
    {
        return ++t;
    }
};

template<typename T>
struct PostIncrement
{
    constexpr auto operator()(T& t) const -> T
    {
        const auto pre = t;
        ++t;
        return pre;
    }
};

} // namespace ECS

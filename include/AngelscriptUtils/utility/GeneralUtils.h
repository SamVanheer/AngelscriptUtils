#pragma once

#include <utility>

namespace asutils
{
//From https://stackoverflow.com/a/38140932; modified to be easier to use
inline std::size_t HashCombineWithSeed(std::size_t seed)
{
    return seed;
}

template <typename T, typename... Rest>
inline std::size_t HashCombineWithSeed(std::size_t seed, const T& v, Rest&&... rest)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    return HashCombineWithSeed(seed, std::forward<Rest>(rest)...);
}

template <typename T, typename... Rest>
inline std::size_t HashCombine(const T& v, Rest&&... rest)
{
    std::hash<T> hasher;
    std::size_t seed = hasher(v) + 0x9e3779b9;

    return HashCombineWithSeed(seed, std::forward<Rest>(rest)...);
}
}

#pragma once
#include "YUME/Debug/instrumentor.h"

#include <functional>



namespace YUME
{
    template <typename T>
    inline void HashCombine(std::size_t& p_Seed, const T& p_Value)
    {
        YM_PROFILE_FUNCTION();

        std::hash<T> hasher;
        p_Seed ^= hasher(p_Value) + 0x9e3779b9 + (p_Seed << 6) + (p_Seed >> 2);
    }

    template <typename T, typename... Rest>
    inline void HashCombine(std::size_t& p_Seed, const T& p_Value, const Rest&... p_Rest)
    {
        YM_PROFILE_FUNCTION();

        HashCombine(p_Seed, p_Value);
        (HashCombine(p_Seed, p_Rest), ...);
    }

} // YUME

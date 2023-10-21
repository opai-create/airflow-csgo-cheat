#pragma once

namespace seeds
{
    static INLINE constexpr std::uint32_t const_xs32_from_seed(std::uint32_t seed, int add = 0)
    {
        seed ^= seed << (13 + add);
        seed ^= seed >> (17 + add);
        seed ^= seed << (15 + add);
        return seed;
    }

    static INLINE std::uint32_t xs32_from_seed(std::uint32_t seed, int add = 0)
    {
        seed ^= seed << (13 + add);
        seed ^= seed >> (17 + add);
        seed ^= seed << (15 + add);
        return seed;
    }
}
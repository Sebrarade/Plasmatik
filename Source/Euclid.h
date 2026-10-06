#pragma once
#include <array>
#include <algorithm>

namespace plasma
{
inline std::array<bool, 16> euclideanPattern (int pulses, int rotation)
{
    std::array<bool, 16> pattern {};
    pulses = std::clamp (pulses, 0, 16);
    rotation = ((rotation % 16) + 16) % 16;

    for (int i = 0; i < 16; ++i)
    {
        const int src = (i - rotation + 16) % 16;
        pattern[(size_t) i] = pulses > 0 && ((src * pulses) % 16) < pulses;
    }
    return pattern;
}
}

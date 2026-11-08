/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** FixedPoint.cpp
*/

#include "FixedPoint.hpp"

#include <cmath>
#include <limits>

namespace rtype::protocol {

std::int16_t toFixed(float pixels) noexcept
{
    constexpr float LOW = std::numeric_limits<std::int16_t>::min();
    constexpr float HIGH = std::numeric_limits<std::int16_t>::max();

    if (std::isnan(pixels)) {
        return 0;
    }
    const float scaled = std::round(pixels * FIXED_POINT_SCALE);

    return static_cast<std::int16_t>(std::fmin(std::fmax(scaled, LOW), HIGH));
}

float fromFixed(std::int16_t value) noexcept
{
    return static_cast<float>(value) / FIXED_POINT_SCALE;
}

} /* namespace rtype::protocol */

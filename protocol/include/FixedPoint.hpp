/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** FixedPoint.hpp
*/

#ifndef FIXEDPOINT_HPP_
#define FIXEDPOINT_HPP_

#include <cstdint>

namespace rtype::protocol {

inline constexpr float FIXED_POINT_SCALE = 16.0F;

/**
 * @brief Converts pixels to 1/16 pixel units.
 *
 * The result is rounded to the nearest unit and saturated to the i16 range.
 * NaN gives 0, so a bad float never reaches the wire.
 */
std::int16_t toFixed(float pixels) noexcept;

/**
 * @brief Converts 1/16 pixel units back to pixels.
 */
float fromFixed(std::int16_t value) noexcept;

} /* namespace rtype::protocol */

#endif /* !FIXEDPOINT_HPP_ */

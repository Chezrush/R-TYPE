/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SerialNumber.hpp
*/

#ifndef SERIALNUMBER_HPP_
#define SERIALNUMBER_HPP_

#include <cstdint>

namespace rtype::protocol {

/**
 * @brief Serial number arithmetic for wrapping counters (seq, rel_id,
 * input_id, tick).
 *
 * a is newer than b when (a - b) modulo 2^n is in 1 .. 2^(n-1) - 1.
 */
bool isNewer(std::uint16_t a, std::uint16_t b) noexcept;
bool isNewer(std::uint32_t a, std::uint32_t b) noexcept;

/**
 * @brief Returns the wrapping distance from b to a, a - b modulo 2^n.
 */
std::uint16_t distance(std::uint16_t a, std::uint16_t b) noexcept;
std::uint32_t distance(std::uint32_t a, std::uint32_t b) noexcept;

} /* namespace rtype::protocol */

#endif /* !SERIALNUMBER_HPP_ */

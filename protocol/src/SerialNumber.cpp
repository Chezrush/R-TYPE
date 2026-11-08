/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SerialNumber.cpp
*/

#include "SerialNumber.hpp"

namespace rtype::protocol {

std::uint16_t distance(std::uint16_t a, std::uint16_t b) noexcept
{
    return static_cast<std::uint16_t>(a - b);
}

std::uint32_t distance(std::uint32_t a, std::uint32_t b) noexcept
{
    return a - b;
}

bool isNewer(std::uint16_t a, std::uint16_t b) noexcept
{
    const std::uint16_t delta = distance(a, b);

    return delta != 0 && delta < 0x8000U;
}

bool isNewer(std::uint32_t a, std::uint32_t b) noexcept
{
    const std::uint32_t delta = distance(a, b);

    return delta != 0 && delta < 0x80000000U;
}

} /* namespace rtype::protocol */

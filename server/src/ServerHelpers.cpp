/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** ServerHelpers
*/

#include "ServerHelpers.hpp"

std::uint16_t ServerHelpers::readUint16BigEndian(const char *data) noexcept
{
    const auto high = static_cast<unsigned char>(data[0]);
    const auto low = static_cast<unsigned char>(data[1]);

    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(high) << 8U) | low);
}

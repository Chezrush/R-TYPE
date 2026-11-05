/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ByteOrder.cpp
*/

#include "ByteOrder.hpp"

namespace rtype::protocol {

std::uint16_t readUint16BigEndian(const char *data) noexcept
{
    const auto high = static_cast<std::uint8_t>(data[0]);
    const auto low = static_cast<std::uint8_t>(data[1]);

    return static_cast<std::uint16_t>((high << 8U) | low);
}

std::uint32_t readUint32BigEndian(const char *data) noexcept
{
    return (static_cast<std::uint32_t>(readUint16BigEndian(data)) << 16U) |
           readUint16BigEndian(data + 2);
}

void writeUint8(std::string &out, std::uint8_t value)
{
    out += static_cast<char>(value);
}

void writeUint16BigEndian(std::string &out, std::uint16_t value)
{
    writeUint8(out, static_cast<std::uint8_t>(value >> 8U));
    writeUint8(out, static_cast<std::uint8_t>(value & 0xFFU));
}

void writeUint32BigEndian(std::string &out, std::uint32_t value)
{
    writeUint16BigEndian(out, static_cast<std::uint16_t>(value >> 16U));
    writeUint16BigEndian(out, static_cast<std::uint16_t>(value & 0xFFFFU));
}

} /* namespace rtype::protocol */

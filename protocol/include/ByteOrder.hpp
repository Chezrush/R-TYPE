/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ByteOrder.hpp
*/

#ifndef BYTEORDER_HPP_
#define BYTEORDER_HPP_

#include <cstdint>
#include <string>

namespace rtype::protocol {

std::uint16_t readUint16BigEndian(const char *data) noexcept;
std::uint32_t readUint32BigEndian(const char *data) noexcept;
void writeUint8(std::string &out, std::uint8_t value);
void writeUint16BigEndian(std::string &out, std::uint16_t value);
void writeUint32BigEndian(std::string &out, std::uint32_t value);

} /* namespace rtype::protocol */

#endif /* !BYTEORDER_HPP_ */

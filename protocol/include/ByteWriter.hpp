/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ByteWriter.hpp
*/

#ifndef BYTEWRITER_HPP_
#define BYTEWRITER_HPP_

#include <cstdint>
#include <string>
#include <string_view>

namespace rtype::protocol {

/**
 * @brief Appends big-endian integers to a byte string.
 */
class ByteWriter {
public:
    explicit ByteWriter(std::string &out) noexcept;

    ByteWriter &writeU8(std::uint8_t value);
    ByteWriter &writeU16(std::uint16_t value);
    ByteWriter &writeU32(std::uint32_t value);
    ByteWriter &writeU64(std::uint64_t value);
    ByteWriter &writeI16(std::int16_t value);
    ByteWriter &writeBytes(std::string_view bytes);

private:
    std::string &_out;
};

} /* namespace rtype::protocol */

#endif /* !BYTEWRITER_HPP_ */

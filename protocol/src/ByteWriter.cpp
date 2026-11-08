/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ByteWriter.cpp
*/

#include "ByteWriter.hpp"

#include "ByteOrder.hpp"

namespace rtype::protocol {

ByteWriter::ByteWriter(std::string &out) noexcept
    : _out(out)
{}

ByteWriter &ByteWriter::writeU8(std::uint8_t value)
{
    writeUint8(_out, value);
    return *this;
}

ByteWriter &ByteWriter::writeU16(std::uint16_t value)
{
    writeUint16BigEndian(_out, value);
    return *this;
}

ByteWriter &ByteWriter::writeU32(std::uint32_t value)
{
    writeUint32BigEndian(_out, value);
    return *this;
}

ByteWriter &ByteWriter::writeU64(std::uint64_t value)
{
    writeU32(static_cast<std::uint32_t>(value >> 32U));
    return writeU32(static_cast<std::uint32_t>(value));
}

ByteWriter &ByteWriter::writeI16(std::int16_t value)
{
    return writeU16(static_cast<std::uint16_t>(value));
}

ByteWriter &ByteWriter::writeBytes(std::string_view bytes)
{
    _out += bytes;
    return *this;
}

} /* namespace rtype::protocol */

/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ByteReader.cpp
*/

#include "ByteReader.hpp"

#include "ByteOrder.hpp"

namespace rtype::protocol {

ByteReader::ByteReader(std::string_view data) noexcept
    : _data(data)
{}

bool ByteReader::take(std::size_t count) noexcept
{
    if (_failed || count > _data.size() - _offset) {
        _failed = true;
        return false;
    }
    return true;
}

std::uint8_t ByteReader::readU8() noexcept
{
    if (!take(1)) {
        return 0;
    }
    return static_cast<std::uint8_t>(_data[_offset++]);
}

std::uint16_t ByteReader::readU16() noexcept
{
    if (!take(2)) {
        return 0;
    }
    const std::uint16_t value = readUint16BigEndian(_data.data() + _offset);

    _offset += 2;
    return value;
}

std::uint32_t ByteReader::readU32() noexcept
{
    if (!take(4)) {
        return 0;
    }
    const std::uint32_t value = readUint32BigEndian(_data.data() + _offset);

    _offset += 4;
    return value;
}

std::uint64_t ByteReader::readU64() noexcept
{
    const std::uint64_t high = readU32();
    const std::uint64_t low = readU32();

    return (high << 32U) | low;
}

std::int16_t ByteReader::readI16() noexcept
{
    return static_cast<std::int16_t>(readU16());
}

std::string_view ByteReader::readBytes(std::size_t count) noexcept
{
    if (!take(count)) {
        return {};
    }
    const std::string_view bytes = _data.substr(_offset, count);

    _offset += count;
    return bytes;
}

bool ByteReader::ok() const noexcept
{
    return !_failed;
}

std::size_t ByteReader::remaining() const noexcept
{
    return _data.size() - _offset;
}

bool ByteReader::finished() const noexcept
{
    return !_failed && _offset == _data.size();
}

} /* namespace rtype::protocol */

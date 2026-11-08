/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ByteReader.hpp
*/

#ifndef BYTEREADER_HPP_
#define BYTEREADER_HPP_

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace rtype::protocol {

/**
 * @brief Bounded cursor over received bytes.
 *
 * A read past the end never touches memory: it returns 0 and makes the
 * reader fail for good. Check ok() once after decoding a whole body.
 */
class ByteReader {
public:
    explicit ByteReader(std::string_view data) noexcept;

    std::uint8_t readU8() noexcept;
    std::uint16_t readU16() noexcept;
    std::uint32_t readU32() noexcept;
    std::uint64_t readU64() noexcept;
    std::int16_t readI16() noexcept;

    /**
     * @brief Takes the next count bytes, an empty view if they are missing.
     */
    std::string_view readBytes(std::size_t count) noexcept;

    [[nodiscard]] bool ok() const noexcept;
    [[nodiscard]] std::size_t remaining() const noexcept;

    /**
     * @brief Returns true if no read failed and every byte was consumed.
     */
    [[nodiscard]] bool finished() const noexcept;

private:
    bool take(std::size_t count) noexcept;

    std::string_view _data;
    std::size_t _offset = 0;
    bool _failed = false;
};

} /* namespace rtype::protocol */

#endif /* !BYTEREADER_HPP_ */

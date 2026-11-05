/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** PacketReader.hpp
*/

#ifndef PACKETREADER_HPP_
#define PACKETREADER_HPP_

#include <cstdint>
#include <optional>
#include <string_view>

#include "Protocol.hpp"

namespace rtype::protocol {

struct Record {
    std::uint8_t type = 0;
    std::string_view body;

    friend bool operator==(const Record &, const Record &) = default;
};

class PacketReader {
public:
    /**
     * @brief Returns true if the header and every record of the packet are
     * valid.
     */
    [[nodiscard]] static bool isValid(std::string_view packet) noexcept;

    /**
     * @brief Reads and checks the header, returns std::nullopt if invalid.
     */
    [[nodiscard]] static std::optional<MessageHeader>
    parseHeader(std::string_view packet) noexcept;

    /**
     * @brief Returns true if a record of the packet has this type.
     */
    [[nodiscard]] static bool hasRecord(std::string_view packet,
                                        MessageType type) noexcept;

    [[nodiscard]] static std::string_view
    recordsOf(std::string_view packet) noexcept;

    /**
     * @brief Reads the first record of remaining and removes it, returns
     * std::nullopt if it is truncated.
     */
    [[nodiscard]] static std::optional<Record>
    readRecord(std::string_view &remaining) noexcept;

private:
    static MessageHeader readHeader(std::string_view packet) noexcept;
    static bool isHeaderValid(const MessageHeader &header) noexcept;
    static bool areRecordsValid(std::string_view packet) noexcept;
    static bool isRecordValid(const Record &record) noexcept;
};

} /* namespace rtype::protocol */

#endif /* !PACKETREADER_HPP_ */

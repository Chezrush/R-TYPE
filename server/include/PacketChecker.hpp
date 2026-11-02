/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** PacketChecker
*/

#ifndef PACKETCHECKER_HPP_
#define PACKETCHECKER_HPP_

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "ServerMessage.hpp"

namespace rtype::server {

class PacketChecker {
public:
    /**
     * @brief Returns true if the header and every record of the packet are
     * valid.
     */
    [[nodiscard]] static bool isValid(const std::string &payload) noexcept;

    /**
     * @brief Reads and checks the header, returns std::nullopt if invalid.
     */
    [[nodiscard]] static std::optional<MessageHeader>
    parsHeader(const std::string &payload) noexcept;

private:
    struct Record {
        std::uint8_t type = 0;
        std::string_view body;
    };

    using BodyCheck = std::function<bool(std::string_view body)>;

    /**
     * @brief Reads the header fields without checking them.
     */
    static MessageHeader readHeader(const std::string &payload) noexcept;

    /**
     * @brief Checks the magic, the version and the flags of a header.
     */
    static bool isHeaderValid(const MessageHeader &header) noexcept;

    /**
     * @brief Returns true if every record after the header is valid.
     */
    static bool parsRecords(const std::string &payload) noexcept;

    /**
     * @brief Reads the first record of remaining and removes it, returns
     * std::nullopt if it is truncated.
     */
    static std::optional<Record>
    readRecord(std::string_view &remaining) noexcept;

    /**
     * @brief Checks the type, the rel_id and the body of a record.
     */
    static bool isRecordValid(const Record &record) noexcept;

    static const std::unordered_map<MessageType, BodyCheck> BODY_CHECKS;
};

} /* namespace rtype::server */

#endif /* !PACKETCHECKER_HPP_ */

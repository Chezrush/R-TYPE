/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MessageCodec.hpp
*/

#ifndef MESSAGECODEC_HPP_
#define MESSAGECODEC_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "Messages.hpp"

namespace rtype::protocol {

/**
 * @brief Converts typed messages to record bodies and back.
 *
 * A body never contains the rel_id of a reliable record: the reliable
 * channel adds and removes it.
 */
class MessageCodec {
public:
    /**
     * @brief Serializes the fields of a message.
     *
     * Values outside the protocol limits are brought back inside: names are
     * truncated and made printable, an input holds at most MAX_INPUTS
     * samples, a snapshot at most MAX_SNAPSHOT_ENTITIES entities.
     */
    [[nodiscard]] static std::string encodeBody(const Message &message);

    /**
     * @brief Reads and validates a body.
     *
     * @return std::nullopt if the type is unknown or the body is invalid.
     */
    [[nodiscard]] static std::optional<Message>
    decodeBody(std::uint8_t type, std::string_view body) noexcept;

    /**
     * @brief Appends the record of an unreliable message to the packet.
     */
    static void appendMessage(std::string &packet, const Message &message);

    /**
     * @brief Appends the record of a reliable message, with its rel_id.
     */
    static void appendReliable(std::string &packet, const Message &message,
                               std::uint16_t relId);

    /**
     * @brief Returns the size of the record that appendMessage() adds.
     */
    [[nodiscard]] static std::size_t recordSize(const Message &message);
};

} /* namespace rtype::protocol */

#endif /* !MESSAGECODEC_HPP_ */

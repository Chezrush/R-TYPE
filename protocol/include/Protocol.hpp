/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Protocol.hpp
*/

#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <cstddef>
#include <cstdint>

namespace rtype::protocol {

inline constexpr std::size_t PACKET_HEADER_SIZE = 16;
inline constexpr std::size_t RECORD_PREFIX_SIZE = 3;
inline constexpr std::size_t REL_ID_SIZE = 2;
inline constexpr std::size_t MAX_PACKET_SIZE = 1200;
inline constexpr std::size_t MAX_RECORDS = 64;
inline constexpr std::size_t MAX_NAME_LENGTH = 16;
inline constexpr std::size_t MAX_INPUTS = 8;
inline constexpr std::size_t MAX_SNAPSHOT_ENTITIES = 128;
inline constexpr std::uint16_t PROTOCOL_MAGIC = 0x5254;
inline constexpr std::uint8_t PROTOCOL_VERSION = 1;
inline constexpr std::uint8_t MAX_PLAYERS = 16;
inline constexpr std::uint8_t NO_PLAYER = 0xFF;
inline constexpr std::uint8_t FLAG_ACK_VALID = 0x01;
inline constexpr std::uint8_t RELIABLE_BIT = 0x80;
inline constexpr std::uint8_t KEYS_MASK = 0x1F;

enum class MessageType : std::uint8_t {
    HELLO = 0x01,
    CHALLENGE = 0x02,
    WELCOME = 0x03,
    REJECT = 0x04,
    GOODBYE = 0x05,
    INPUT = 0x10,
    SNAPSHOT = 0x11,
    SPAWN = 0x81,
    DESTROY = 0x82,
    PLAYERJOINED = 0x83,
    PLAYERLEFT = 0x84,
    SCORE = 0x85,
    GAMESTATE = 0x86
};

/**
 * @brief Values of goodbye.reason, also used by player_left.reason.
 */
enum class DisconnectReason : std::uint8_t {
    QUIT = 0,
    TIMEOUT = 1,
    SHUTDOWN = 2,
    PROTOCOL = 3,
    QUEUE_OVERFLOW = 4
};

struct MessageHeader {
    std::uint16_t magicNumber = PROTOCOL_MAGIC;
    std::uint8_t version = PROTOCOL_VERSION;
    std::uint8_t flags = 0;
    std::uint32_t session = 0;
    std::uint16_t seq = 0;
    std::uint16_t ack = 0;
    std::uint32_t ackBits = 0;

    bool operator==(const MessageHeader &) const = default;
};

} /* namespace rtype::protocol */

#endif /* !PROTOCOL_HPP_ */
